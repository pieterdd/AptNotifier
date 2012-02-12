#include "inputbox.h"

#include <QPushButton>

InputBox::InputBox(const QString& description, const QRegExp &validator, QWidget *parent) :
    QDialog(parent)
{
    setupGUI();
    evaluateValue();

    // Set up instruction prompt and validator
    _lblDescription.setText(description);
    _validator = validator;
}

QString InputBox::inputValue()
{
    return _txtInput.text();
}

void InputBox::setupGUI()
{
    // Text input validation
    connect(&_txtInput, SIGNAL(textChanged(QString)), this, SLOT(evaluateValue()));

    // Button group: add OK and Cancel buttons
    _buttons.addButton(QDialogButtonBox::Cancel);
    _buttons.addButton(QDialogButtonBox::Ok);
    connect(&_buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(&_buttons, SIGNAL(rejected()), this, SLOT(reject()));

    // Configure the main layout
    _mainLayout.addWidget(&_lblDescription);
    _mainLayout.addWidget(&_txtInput);
    _mainLayout.addWidget(&_buttons);
    setLayout(&_mainLayout);
}

void InputBox::evaluateValue()
{
    QString value = _txtInput.text();

    // Adjust button state accordingly
    if (_validator.exactMatch(value) && value != "")
        _buttons.button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        _buttons.button(QDialogButtonBox::Ok)->setEnabled(false);
}
