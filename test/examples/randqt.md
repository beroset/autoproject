# [GUI Number Generator in QT C++](https://codereview.stackexchange.com/questions/217257)
### tags: ['c++', 'gui', 'qt']

I'm pretty new to Qt but already have a little experience with C++ so the first "project" I wanted to try out was a ***GUI random number generator***. 
I am looking for some advices either in naming conventions or OOP architecture, what should I do better and what I'm already doing good. The code is really short so there is not so much to review but I hope you will find something.

1) I know that you should NOT use `_` prefix, but I really like it and `m_` looks ugly to me. 

2) I could not decide whether it's good to write code like this ( I find this better ):

    _maxSpinBox->setMinimum    ( Config::SpinBox::minimum );
    _maxSpinBox->setMaximum    ( Config::SpinBox::maximum );
    _maxSpinBox->setSingleStep ( Config::SpinBox::single_step );
    _maxSpinBox->setValue      ( Config::SpinBox::default_value );
     
Or like this
     
    _maxSpinBox->setMinimum ( Config::SpinBox::minimum );
    _maxSpinBox->setMaximum ( Config::SpinBox::maximum );
    _maxSpinBox->setSingleStep ( Config::SpinBox::single_step );
    _maxSpinBox->setValue ( Config::SpinBox::default_value );

 
3) I also thought about adding `using namespace Config;` into `generator.cpp` because writing `Config::` everywhere is really annoying.

Thanks for tips

[![enter image description here][1]][1]



***main.cpp***

***

    #include <QApplication>
    #include <iostream>
    #include "generator.h"
    int main(int argc, char **argv)
    {
        QApplication qapp( argc, argv );
        Generator generator{};
        generator.show ();
        try {
            qapp.exec();
        }
        catch(const std::exception& e) {
            std::cerr << e.what () << std::endl;
        }
        return 0;
    }

***config.h***

    #ifndef CONFIG_H
    #define CONFIG_H
    #include <QFont>
    #include <QString>
    namespace Config
    {
    
    namespace Window
    {
        constexpr static int height = 150;
        constexpr static int width  = 300;
    } // Window
    
    namespace Button
    {
        const static QString title  = "Generate";
        constexpr static int height = 30;
        constexpr static int width  = 80;
        constexpr static int pos_x  = Window::width  / 2 - width  / 2;
        constexpr static int pos_y  = Window::height - height - 10;
    } // Button
    
    namespace Display
    {
        constexpr static int height        = 45;
        constexpr static int width         = 90;
        constexpr static int pos_x         = Window::width / 2 - width / 2;
        constexpr static int pos_y         = 20;
        constexpr static int default_value = 0;
    } // Display
    
    namespace Fonts
    {
        const static QFont serifFont( "Times", 10, QFont::Bold );
        const static QFont sansFont( "Helvetica [Cronyx]", 12 );
    } // Fonts
    
    namespace SpinBox
    {
        constexpr static int minimum       = -30000;
        constexpr static int maximum       = 30000;
        constexpr static int single_step   = 1;
        constexpr static int default_value = 0;
    } // SpinBox
    
    } // Config
    
    
    
    #endif // CONFIG_H

***generator.h***

    #ifndef GENERATOR_H
    #define GENERATOR_H
    
    #include <QWidget>
    #include <exception>
    class QPushButton;
    class QLabel;
    class QSpinBox;
    class QGroupBox;
    class QVBoxLayout;
    
    struct BadParameters : std::logic_error
    {
        using std::logic_error::logic_error;
    };
    
    class Generator : public QWidget
    {
        Q_OBJECT
    public:
        explicit Generator( QWidget* parent = nullptr );
    public slots:
        void showNumber();
    signals:
    
    private:
        QPushButton* _button;
        QLabel*      _display;
        QSpinBox*    _minSpinBox;
        QSpinBox*    _maxSpinBox;
        QGroupBox*   _groupBox;
        QVBoxLayout* _layout;
        int          _generateNumber( int low, int high );
        void         _createSpinBoxes();
        void         _createMinSpinBox();
        void         _createMaxSpinBox();
        void         _createSpinBoxLayout();
        void         _createButton();
        void         _createDisplay();
        void         _init();
    };
    
    #endif // GENERATOR_H

***generator.cpp***

    #include <QGroupBox>
    #include <QLabel>
    #include <QPushButton>
    #include <QSpinBox>
    #include <QTime>
    #include <QVBoxLayout>
    
    #include <random>
    #include <qglobal.h>
    
    #include "config.h"
    #include "generator.h"
    
    
    Generator::Generator( QWidget* parent )
        : QWidget( parent )
    {
        _init();
        _createDisplay ();
        _createButton ();
        _createSpinBoxes ();
        connect ( _button, SIGNAL(clicked()), this, SLOT(showNumber()) );
    }
    void Generator::_init() {
        QTime time = QTime::currentTime ();
        qsrand( static_cast< uint >( time.msec ()) );
        setFixedSize( Config::Window::width, Config::Window::height );
        setWindowTitle( "Random Number Generator" );
    }
    void Generator::_createButton() {
        _button = new QPushButton( Config::Button::title, this );
        _button->setGeometry ( Config::Button::pos_x,
                               Config::Button::pos_y,
                               Config::Button::width,
                               Config::Button::height );
    }
    void Generator::_createDisplay() {
         _display = new QLabel( this );
         _display->setFont      ( Config::Fonts::sansFont );
         _display->setAlignment ( Qt::AlignCenter);
         _display->setGeometry  ( Config::Display::pos_x,
                                  Config::Display::pos_y,
                                  Config::Display::width,
                                  Config::Display::height );
    
         _display->setNum ( Config::Display::default_value );
    }
    void Generator::_createSpinBoxes() {
        _createMinSpinBox();
        _createMaxSpinBox();
        _createSpinBoxLayout();
    }
    void Generator::_createSpinBoxLayout(){
        _groupBox        = new QGroupBox( this );
        _layout          = new QVBoxLayout;
        QLabel* labelMin = new QLabel( tr("Minimum: ") );
        QLabel* labelMax = new QLabel( tr("Maximum: ") );
    
        _layout->addWidget   ( labelMin );
        _layout->addWidget   ( _minSpinBox );
        _layout->addWidget   ( labelMax );
        _layout->addWidget   ( _maxSpinBox );
        _groupBox->setLayout ( _layout );
    }
    void Generator::_createMaxSpinBox() {
        _maxSpinBox = new QSpinBox ( this );
        _maxSpinBox->setMinimum    ( Config::SpinBox::minimum );
        _maxSpinBox->setMaximum    ( Config::SpinBox::maximum );
        _maxSpinBox->setSingleStep ( Config::SpinBox::single_step );
        _maxSpinBox->setValue      ( Config::SpinBox::default_value );
    }
    void Generator::_createMinSpinBox() {
        _minSpinBox = new QSpinBox ( this );
        _minSpinBox->setMinimum    ( Config::SpinBox::minimum );
        _minSpinBox->setMaximum    ( Config::SpinBox::maximum );
        _minSpinBox->setSingleStep ( Config::SpinBox::single_step );
        _minSpinBox->setValue      ( Config::SpinBox::default_value );
    }
    int Generator::_generateNumber( int low, int high ) {
    
        if ( low > high ) {
            throw BadParameters( "Upper bound is NOT higher \n" );
        }
        return qrand() % (( high + 1) - low) + low;
    }
    void Generator::showNumber() {
        _display->setNum( _generateNumber( _minSpinBox->value(),
                                           _maxSpinBox->value () ));
    }


This


  [1]: https://i.stack.imgur.com/oyDsY.png