#include "worker.h"
#include "main.h"

#include "../../../libusb/libusb/libusb.h"
#include "../../../core/core.h"
#include "../../../../shared/enums.h"
#include "../../../../shared/cmddesc.h"

// static struct libusb_transfer *transfer_out = nullptr;
static struct libusb_transfer *transfer_in = nullptr;
static libusb_device_handle *handle = nullptr;
static libusb_context *ctx = nullptr;

// static std::mutex usbsmutex;

static glue glue_;
static Worker *wptr;

// Local buffer for receiving USB data
static uint8_t in_buffer[ADAPTER_BUFzOUT];

// Async. USB thread has to know when it's time to quit
static volatile bool run_USBThread;

// Spawned threads has to know file name and which target to perform actions on
static int index_;
static QString fname_;

// Constructors and destructors
Worker::Worker() {}
Worker::~Worker() {}

////////////////////////////////////////////////////
/// Err.. Don't ask!
// Only reason for conversion is to preserve the message when it's transferred between threads
void Worker::WrkMsg_push(QString msg)
{   glue_.CastMessage(msg.toUtf8()); }
void Worker::WrkProg_push(uint prog)
{   glue_.CastProgress(prog); }

void Worker::WrkMsg_inter(const char *msg)
{   emit WrkMsg_emit(QString::fromStdString(msg)); }
void Worker::WrkProg_inter(uint prog)
{   emit WrkProg_emit(prog); }

static void MessagePoint(const char *msg)
{   wptr->WrkMsg_inter(msg); }
static void ProgressPoint(uint prog)
{   wptr->WrkProg_inter(prog); }

////////////////////////////////////////////////////
/// USB functions
static void cb_in(struct libusb_transfer *transfer)
{
    core_HandleRecData(transfer->buffer, static_cast<uint32_t>(transfer->actual_length));
    libusb_submit_transfer(transfer);
}

static void cb_out(struct libusb_transfer *transfer)
{
    libusb_free_transfer(transfer);
    // usbsmutex.unlock();
    // qDebug() << "Mutex unlocked";
}

static void USBProcess()
{
    // qDebug() << "Async USB thread ID:" << QThread::currentThreadId();
    run_USBThread = true;
    while (run_USBThread)
    {
        libusb_handle_events_completed(ctx, nullptr);
        // libusb_handle_events(ctx);
    }
}

// This is.. let's say a particularly stupid thing to do ;)
static void usb_SendArr(void *ptr, uint32_t noBytes)
{
    // qDebug() << "Thread ID trying to send:" << QThread::currentThreadId();
    // Our callback will unlock this
    // usbsmutex.lock();
    // qDebug() << "Mutex locked";

    libusb_transfer *transfer_out = libusb_alloc_transfer(0);
    libusb_fill_bulk_transfer(
                transfer_out,           // Transfer
                handle,                 // Device handle
                LIBUSB_ENDPOINT_OUT | 3,// Endpoint
                reinterpret_cast<uint8_t*>(ptr),// Send buffer
                static_cast<int>(noBytes),// Size out
                cb_out,                 // Callback
                nullptr,                // user data to pass to callback function
                8000);                  // Timeout

    // Queue it and forget it
    libusb_submit_transfer(transfer_out);
}

static bool usb_open()
{
    // int kernelDriverDetached = 0;
    int res;

    if ( libusb_init(nullptr)!= 0 )
    {
        core_castText("Could not initialize libusb");
        return false;
    }

    handle = libusb_open_device_with_vid_pid(ctx, 0xFFFF, 0x0107);
    if (!handle)
    {
        core_castText("Unable to open device");
        return false;
    }

    if (libusb_kernel_driver_active(handle, 0))
    {
        res = libusb_detach_kernel_driver(handle, 0);
        if (res == 0)
        {
            // kernelDriverDetached = 1;
        }
        else
        {
            core_castText("Error detaching kernel driver");
            return false;
        }
    }

    if (libusb_claim_interface(handle, 0) != 0)
    {
        core_castText("Error claiming interface");
        return false;
    }

    transfer_in = libusb_alloc_transfer(0);

    libusb_fill_bulk_transfer(
                transfer_in,            // Transfer
                handle,                 // Device handle
                LIBUSB_ENDPOINT_IN | 1, // Endpoint
                in_buffer,              // Receive buffer
                ADAPTER_BUFzOUT,        // Size in
                cb_in,                  // Callback
                nullptr,                // user data to pass to callback function
                0);                     // Timeout

    libusb_submit_transfer(transfer_in);

    return true;
}

void Worker::DeInitUSB()
{
    // Tell our thread that it's time to exit
    // libusb_handle_events() will exit race condition, if present, once libusb has detached
    run_USBThread = false;

    core_InstallSendArray(nullptr);
    libusb_close(handle);
    libusb_release_interface(handle,0);
    handle = nullptr;

    core_castText("Device detached");
}

bool Worker::InitUSB()
{
    // usbsmutex.unlock();

    if (usb_open())
    {
        core_castText("Device attached");
        return true;
    }
    return false;
}

////////////////////////////////////////////////////
/// Glue..
static bool SaveBufferToFile(int Size)
{
    const void *bufptr = core_ReturnBufferPointer();
    if (bufptr)
    {
        QFile file(fname_);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(reinterpret_cast<const char*>(bufptr), Size);
            file.waitForBytesWritten(10000);
            file.close();


            uint32_t checksum = 0;
            const uint8_t *ptr = reinterpret_cast<const unsigned char*>(reinterpret_cast<const char*>(bufptr));


            for (int i = 0; i < Size; i++)
                checksum += *ptr++;

            core_castText("Buffer length: %08X", Size);
            core_castText("Checksum     : %08X", checksum);
            return true;
        }
        else
            core_castText("Error: Could not open file for saving");
    }

    // No buffer pointer was given due to previous fault
    else
        core_castText(core_TranslateFault());
    return false;
}

// Call this BEFORE making other calls to core
static void InstallPointers(Worker *classptr)
{
    wptr = classptr;
    core_InstallSendArray(reinterpret_cast<void*>(&usb_SendArr)   );
    core_InstallMessage(  reinterpret_cast<void*>(&MessagePoint)  );
    core_InstallProgress( reinterpret_cast<void*>(&ProgressPoint) );
}

////////////////////////////////////////////////////
/// Thread workers

void Worker::DumpEepromProcess()
{
    // qDebug() << "Dump thread id:" << QThread::currentThreadId();
    QElapsedTimer tim;

    InstallPointers(this);
    ProgressPoint(0);
    tim.start();

    if (!InitUSB())
    {
        emit finished();
        return;
    }

    // Spawn another thread for USB
    std::thread t1(USBProcess);

    core_DumpEEPROM(static_cast<uint>(index_));
    if (core_ReturnFaultStatus())
        core_castText("EEPROM dump failed");
    else
    {
        if (SaveBufferToFile( static_cast<int>(core_TargetSizeEEPROM(static_cast<uint>(index_)))))
            core_castText("EEPROM dump successful");
    }

    DeInitUSB();
    t1.join();

    long long time = tim.elapsed();
    double speed = (core_TargetSizeEEPROM(static_cast<uint>(index_))/1024) / (time/1000.0);
    core_castText("Took: %u mS (%1.3f KB/S)", time, speed);
    emit finished();
}

void Worker::FlashEepromProcess()
{
    // qDebug() << "Flash id:" << QThread::currentThreadId();
    QElapsedTimer tim;
    uint32_t index = static_cast<uint32_t>(index_);

    InstallPointers(this);
    ProgressPoint(0);
    tim.start();

    QFile file(fname_);
    if (file.open(QIODevice::ReadOnly))
    {
        if (file.size() != core_TargetSizeEEPROM(index))
        {
            file.close();
            core_castText("File size does not match target");
            emit finished();
            return;
        }

        const QByteArray qarr = file.readAll();
        file.close();

        if (!InitUSB())
        {
            emit finished();
            return;
        }

        // Spawn another thread for USB
        std::thread t1(USBProcess);

        core_WriteEEPROM(static_cast<uint>(index_), const_cast<char*>(qarr.data()));
        if (core_ReturnFaultStatus())
            core_castText("EEPROM write failed");
        else
            core_castText("EEPROM write successful");

        DeInitUSB();
        t1.join();
    }
    else
        core_castText("Error: Could not open file for reading");

    long long time = tim.elapsed();
    double speed = (core_TargetSizeEEPROM(index)/1024) / (time/1000.0);
    core_castText("Took: %u mS (%1.3f KB/S)", time, speed);
    emit finished();
}

void Worker::DumpProcess()
{
    // qDebug() << "Dump thread id:" << QThread::currentThreadId();
    QElapsedTimer tim;

    InstallPointers(this);
    ProgressPoint(0);
    tim.start();

    if (!InitUSB())
    {
        emit finished();
        return;
    }

    // Spawn another thread for USB
    std::thread t1(USBProcess);

    core_DumpFLASH(static_cast<uint>(index_));
    if (core_ReturnFaultStatus())
        core_castText("Dump failed");
    else
    {
        if (SaveBufferToFile( static_cast<int>(core_TargetSizeFLASH(static_cast<uint>(index_)))))
            core_castText("Dump successful");
    }

    DeInitUSB();
    t1.join();

    long long time = tim.elapsed();
    double speed = (core_TargetSizeFLASH(static_cast<uint>(index_))/1024) / (time/1000.0);
    core_castText("Took: %u mS (%1.3f KB/S)", time, speed);
    emit finished();
}

void Worker::FlashProcess()
{
    // qDebug() << "Flash id:" << QThread::currentThreadId();
    QElapsedTimer tim;
    uint32_t index = static_cast<uint32_t>(index_);

    InstallPointers(this);
    ProgressPoint(0);
    tim.start();

    QFile file(fname_);
    if (file.open(QIODevice::ReadOnly))
    {
        if (file.size() != core_TargetSizeFLASH(index))
        {
            file.close();
            core_castText("File size does not match target");
            emit finished();
            return;
        }

        const QByteArray qarr = file.readAll();
        file.close();

        if (!InitUSB())
        {
            emit finished();
            return;
        }

        // Spawn another thread for USB
        std::thread t1(USBProcess);

        core_FLASH(static_cast<uint>(index_), const_cast<char*>(qarr.data()));
        if (core_ReturnFaultStatus())
            core_castText("Flash failed");
        else
            core_castText("Flash successful");

        DeInitUSB();
        t1.join();
    }
    else
        core_castText("Error: Could not open file for reading");

    long long time = tim.elapsed();
    double speed = (core_TargetSizeFLASH(index)/1024) / (time/1000.0);
    core_castText("Took: %u mS (%1.3f KB/S)", time, speed);
    emit finished();
}

void Worker::WorkerDone()
{
    WrkMsg_push("Thread gone *poof*");
    WrkMsg_push(" ");
    // btnDumpClick() / btnFlashClick() disables input so we have to manually enable it again
    glue_.ECUIndexLogic(index_);
}

////////////////////////////////////////////////////
/// More glue...
// There's a known bug in Qt. XCB error: 3 is likely not my fault in case you see it in the logs
void FileDialog::OpenDialog()
{
    fname =  QFileDialog::getOpenFileName(
        this,
        "Open File",
        QDir::homePath(),
        "Binary files (*.bin) ;; All files (*.*)");
}
void FileDialog::SaveDialog()
{
    fname =  QFileDialog::getSaveFileName(
        this,
        "Save File",
        QDir::homePath(),
        "Binary files (*.bin) ;; All files (*.*)");
}

void Worker::PrepareThread(int index, QString fname, const char *slot)
{
    index_ = index;
    fname_ = fname;

    QThread* thread = new QThread;
    Worker* worker = new Worker();
    worker->moveToThread(thread);

    connect(thread, SIGNAL(started()), worker, slot);
    connect(worker, SIGNAL(WrkMsg_emit(QString)), this, SLOT(WrkMsg_push(QString)));
    connect(worker, SIGNAL(WrkProg_emit(uint)), this, SLOT(WrkProg_push(uint)));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), this, SLOT(WorkerDone()));

    thread->start();
}

void Worker::StartEepromFlash(int index)
{
    FileDialog fd;
    fd.OpenDialog();

    if (fd.fname != nullptr)
        PrepareThread(index, fd.fname, SLOT(FlashEepromProcess()));
    // btnFlashClick() disables input so we have to manually enable it again
    else
        glue_.ECUIndexLogic(index);
}

void Worker::StartEepromDump(int index)
{
    FileDialog fd;
    fd.SaveDialog();

    if (fd.fname != nullptr)
        PrepareThread(index, fd.fname, SLOT(DumpEepromProcess()));
    // btnDumpClick() disables input so we have to manually enable it again
    else
        glue_.ECUIndexLogic(index);
}

void Worker::StartDump(int index)
{
    FileDialog fd;
    fd.SaveDialog();

    if (fd.fname != nullptr)
        PrepareThread(index, fd.fname, SLOT(DumpProcess()));
    // btnDumpClick() disables input so we have to manually enable it again
    else
        glue_.ECUIndexLogic(index);
}

void Worker::StartFlash(int index)
{
    FileDialog fd;
    fd.OpenDialog();

    if (fd.fname != nullptr)
        PrepareThread(index, fd.fname, SLOT(FlashProcess()));
    // btnFlashClick() disables input so we have to manually enable it again
    else
        glue_.ECUIndexLogic(index);
}
