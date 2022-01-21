#include "debugwindow.h"
#include "ui_debugwindow.h"
#include "main.h"
#include "../../../core/core.h"

// Gotta output some crap to the main window, doh.
static glue glue_;

// Total number of registers. Each new element is assigned a number to ease triggers
static uint noRegs = 0;

// Currently selected register
static uint selReg = 0;

// How not to allocate and deallocate items.
static void *HBoxArr = nullptr;
static uint HBoxItems = 0;
static void *RadioArr = nullptr;
static uint RadioItems = 0;
static void *RadioRegArr = nullptr;
static uint RadioRegItems = 0;

void registerBtn::actRegisterBtn(QRadioButton *parent, uint32_t index)
{
    m_index = index;
    connect(parent, SIGNAL (released()),this, SLOT (btnRadioSlot()));
}

void registerBtn::btnRadioSlot()
{
    QString text = "Selected reg index: " + QString().sprintf("%d", m_index);
    glue_.CastMessage(text.toUtf8());
    selReg = m_index;
}

static bool AllocateHBox(QLayout *ptr)
{
    HBoxArr = realloc(HBoxArr, sizeof(QLayout*) * ( HBoxItems + 1 ) );
    if (HBoxArr == nullptr) return false;
    reinterpret_cast<QLayout **> (HBoxArr)[HBoxItems++] = ptr;
    return true;
}

static bool AllocateButton(QRadioButton *ptr)
{
    RadioArr = realloc(RadioArr, sizeof(QRadioButton*) * ( RadioItems + 1 ) );
    if (RadioArr == nullptr) return false;
    reinterpret_cast<QRadioButton **> (RadioArr)[RadioItems++] = ptr;
    return true;
}

static bool AllocateRegisterButton(registerBtn *ptr)
{
    RadioRegArr = realloc(RadioRegArr, sizeof(registerBtn*) * ( RadioRegItems + 1 ) );
    if (RadioRegArr == nullptr) return false;
    reinterpret_cast<registerBtn **> (RadioRegArr)[RadioRegItems++] = ptr;
    return true;
}

// Bruteforce destruction of items.
static void DeAllocateItems()
{
    if (RadioRegArr != nullptr && RadioRegItems) {
        registerBtn** regRowArray = reinterpret_cast<registerBtn **> (RadioRegArr);
        while (RadioRegItems)
            delete regRowArray[--RadioRegItems];
        RadioRegArr = nullptr;
    }
    if (RadioArr != nullptr && RadioItems) {
        QRadioButton** regRowArray = reinterpret_cast<QRadioButton **> (RadioArr);
        while (RadioItems)
            delete regRowArray[--RadioItems];
        RadioArr = nullptr;
    }
    if (HBoxArr != nullptr && HBoxItems) {
        QLayout** regRowArray = reinterpret_cast<QLayout **> (HBoxArr);
        while (HBoxItems)
            delete regRowArray[--HBoxItems];
        HBoxArr = nullptr;
    }

    noRegs = 0;
}

void debugwindow::InsertRegisterRow(const void *rSetPtr, uint32_t noItems)
{
    // uint32_t noItems = *(reinterpret_cast<uint32_t *>(reinterpret_cast<void **>(rSetPtr)[0]));
    uint32_t tempItems = noItems > 16 ? 16 : noItems;
    uint32_t currItem = 0;

    int curVBoxH = ui->widget->size().height();
    int curVBoxV = ui->widget->size().width();

    while (noItems)
    {
        // Create a new horizontal box for every set of registers
        QLayout *hBox = new QHBoxLayout();
        if (!AllocateHBox(hBox))
            return;

        hBox->setParent(ui->regLayout); // Owned by the register layout
        ui->regLayout->addItem(hBox);   // Aaand send it there

        // If a processor has a sh!tton of registers, make sure not to put them all on the same line!
        uint32_t rowItems = noItems > 16 ? 16 : noItems;

        for (uint32_t i = 0; i < rowItems; i++)
        {
            QRadioButton *btn = new QRadioButton( reinterpret_cast<char *>
                                                  (reinterpret_cast<void **>
                                                   (const_cast<void *>(rSetPtr))[currItem++] ), this);
            hBox->addWidget(btn);

            registerBtn *call = new registerBtn();
            call->actRegisterBtn(btn,noRegs++);

            if (!AllocateRegisterButton(call) || !AllocateButton(btn))
                return;
        }

        curVBoxH += 20;
        noItems -= rowItems;
    }

    // If we added more than six radio buttons on one line, make sure to resize window if need be
    if (tempItems > 6)
    {
        int vBoxTmp = 321;
        vBoxTmp += (tempItems-6) * 58;
        curVBoxV = vBoxTmp > curVBoxV ? vBoxTmp : curVBoxV;
    }

    ui->widget->resize(curVBoxV, curVBoxH);
    this->adjustSize();
}

debugwindow::debugwindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::debugwindow)
{
    ui->setupUi(this);
}

void debugwindow::Populate(int index)
{
    ui->dataEdit->clear();
    ui->dataEdit->insert("0x0000000000000000");

    DeAllocateItems();

    const void *rowptr = debug_RegisterList(static_cast<uint>(index));
    if (rowptr)
        InsertRegisterRow(rowptr, debug_noRegisters(static_cast<uint>(index)));
}

debugwindow::~debugwindow()
{
    DeAllocateItems();
    delete ui;
}

void debugwindow::closeEvent(QCloseEvent *event)
{
    DeAllocateItems();
    event->accept();
}
