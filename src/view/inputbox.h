/**
  * \class InputBox
  * \author Pieter De Decker
  *
  * \brief Simple dialog asking for text input. A regex can be supplied for form validation.
  */
#ifndef INPUTBOX_H
#define INPUTBOX_H

#include <QLabel>
#include <QDialog>
#include <QRegExp>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QDialogButtonBox>

class InputBox : public QDialog
{
    Q_OBJECT
public:
    explicit InputBox(const QString& description, const QRegExp& validator = QRegExp("*"), QWidget* parent = NULL);

    /** Returns the value in the text box. */
    QString inputValue();
private:
    /** Configures all controls and layouts. */
    void setupGUI();

    // Controls
    QVBoxLayout _mainLayout;
    QLabel _lblDescription;
    QLineEdit _txtInput;
    QDialogButtonBox _buttons;

    // Text input validator
    QRegExp _validator;
private slots:
    /** Evaluates the validity of the text input based on the validator. */
    void evaluateValue();
};

#endif // INPUTBOX_H
