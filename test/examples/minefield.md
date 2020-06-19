# [Minefield - A Cross Platform Minesweeper clone](https://codereview.stackexchange.com/questions/231598)

[![enter image description here][1]][1]

[![enter image description here][2]][2]

[1]://i.stack.imgur.com/OqLNg.png
[2]://i.stack.imgur.com/Yp9Aq.png

I just finished a full minesweeper clone which is based on [Minesweeper for Windows 95](http://www.minesweeper.info/downloads/Winmine95.html).


I used C++ and Qt to realize it. The full source code can be find in [my git hub](https://github.com/sandro4912/Minefield).


Since this is too much code too review, I want to focus on the logic of the cells. For that I created the following classes:

 - `Cell`: Represents a single cell in the `Minefield`
 - `CellInputHandler`: This `QEventfilter` gets installed on each `Cell` to manage movements and right and left clicks with the cells. 
 - `Minefield`: Manages all the cells' events and tells the game when important events happen (bomb hit, uncovered all safe cells, uncovered first cell).


You wonder why `CellInputHandler` handles the events and not each individual `Cell` directly. Minesweeper supports the following movement which could not be directly implemented in a single cell:

- press left mouse button on `Cell` A and hold it.
- Move out of `Cell` A  ⟶ `Cell` A gets "unpressed" on move out
- Move into `Cell` B ⟶ `Cell` B gets "pressed"


In the Cells directly implemented, I could not detect moving into the Cell with a pressed Mouse Button. So an event filter was the solution. Still, I feel this is a strange solution because I have to make it a friend of `Cell` and friends are often a bad design choice.

So please let me know what you think about the code. Also tell me any other bad practices or improvements I could have made.

Feel free to also check the other classes in the repo. If there's anything strange, let me know.


###cell.h

    #ifndef CELL_H
    #define CELL_H
    
    #include <QWidget>
    
    #include <QElapsedTimer>
    #include <QTimer>
    
    class CellInputHandler;
    
    class Cell : public QWidget
    {
        Q_OBJECT
    public:
        enum class State{
            empty,
            mine
        };
    
        Cell(State state, QWidget *parent = nullptr);
    
        void setCountOfNeighbourMines(int count);
        [[nodiscard]] int countOfNeighbourMines() const;
    
        [[nodiscard]] bool hasMine() const;
        [[nodiscard]] bool hasQuestionmark() const;
        [[nodiscard]] bool isCovered() const;
        [[nodiscard]] bool isFLagged() const;
        [[nodiscard]] bool isPressed() const;
        [[nodiscard]] bool neighbourHasMine() const;
        [[nodiscard]] bool neighbourIsFlagged() const;
    
    public slots:
        void toggleColor(bool value);
        void toggleNewQuestionMarks(bool value);
    
        void increaseCountOfFlaggedNeighbours();
        void decreaseCountOfFlaggedNeighbours();
        void uncoverIfCoveredAndNoMine();
        void uncoverIfNotFlagged();
    
        void pressIfCoveredOrQuestionmark();
        void releaseIfCoveredOrQuestionmarkPressed();
    
        void showMine();
        void setToFlaggedWrong();
    
    signals:
        void hitMine();
        void flagged();
        void unflagged();
        void uncovered();
        void uncoveredEmptyCell();
        void uncoverAreaWithNoMines();
        void uncoverNotFlaggedNeighbours();
        void pressed();
        void released();
        void pressNeighbours();
        void releaseNeighbours();
    
    protected:
        void paintEvent(QPaintEvent *event) override;
    
    private slots:
        void mark();
    
    private:
        enum class DisplayType{
            covered,
            coveredPressed,
            neigboursHave0Mines,
            neigboursHave1Mine,
            neigboursHave2Mines,
            neigboursHave3Mines,
            neigboursHave4Mines,
            neigboursHave5Mines,
            neigboursHave6Mines,
            neigboursHave7Mines,
            neigboursHave8Mines,
            questionmark,
            questionmarkPressed,
            flagged,
            mine,
            mineExploded,
            flaggedWrong,
        };
    
        QImage displayImage(DisplayType type);
    
        void uncover();
        void uncoverMine();
        void setToUncoveredDisplayType();
    
        friend class CellInputHandler;
    
        void handleMousePressEvent(QMouseEvent *event);
        void handleMouseReleaseEvent(QMouseEvent *event);
    
        void handleMouseMoveEventInsideLeftButton(QMouseEvent *event);
        void handleMouseMoveEventOutsideLeftButton(QMouseEvent *event);
    
        void handleMouseMoveEventInsideBothButtons(QMouseEvent *event);
        void handleMouseMoveEventOutsideBothButtons(QMouseEvent *event);
    
        const bool mHasMine;
        bool mNeighboursPressed;
        bool mQuestionMarksOn;
        bool mColorOn;
        int mCountOfNeighbourMines;
        int mCountOfNeigboursFlagged;
        QElapsedTimer mElapsedTimer;
        QTimer mSingleMouseTimerLeft;
        QTimer mSingleMouseTimerRight;
        DisplayType mDisplayType;
    };
    
    #endif // CELL_H


###cell.cpp


    #include "cell.h"
    
    #include "converttograyscale.h"
    #include "cellinputhandler.h"
    
    #include <QApplication>
    #include <QIcon>
    #include <QPainter>
    #include <QStylePainter>
    #include <QStyleOptionButton>
    #include <QMouseEvent>
    #include <QImage>
    
    #include <QDebug>
    
    Cell::Cell(Cell::State state, QWidget *parent)
        :QWidget{ parent },
          mHasMine{ static_cast<bool>(state) },
          mNeighboursPressed{ false },
          mQuestionMarksOn{ true },
          mColorOn{ true },
          mCountOfNeighbourMines{ 0 },
          mCountOfNeigboursFlagged{ 0 },
          mDisplayType{ DisplayType::covered }
    {
        setFixedSize(displayImage(mDisplayType).size());
    
        mElapsedTimer.start();
    
        constexpr auto intervall = 50;
        for(QTimer* timer : {&mSingleMouseTimerRight, &mSingleMouseTimerLeft}){
            timer->setInterval(intervall);
            timer->setSingleShot(true);
        }
    
        connect(&mSingleMouseTimerLeft, &QTimer::timeout,
                this, &Cell::pressIfCoveredOrQuestionmark);
        connect(&mSingleMouseTimerRight, &QTimer::timeout,
                this, &Cell::mark);
    
        setMouseTracking(true);
    }
    
    void Cell::setCountOfNeighbourMines(int count)
    {
        constexpr auto minNeighbourMines = 0;
        constexpr auto maxNeighbourMines = 8;
    
        Q_ASSERT(count >= minNeighbourMines && count <= maxNeighbourMines);
    
        mCountOfNeighbourMines = count;
    }
    
    int Cell::countOfNeighbourMines() const
    {
        return mCountOfNeighbourMines;
    }
    
    bool Cell::hasMine() const
    {
        return mHasMine;
    }
    
    bool Cell::hasQuestionmark() const
    {
        return mDisplayType == DisplayType::questionmark;
    }
    
    bool Cell::isCovered() const
    {
        return mDisplayType == DisplayType::covered;
    }
    
    bool Cell::isFLagged() const
    {
        return mDisplayType == DisplayType::flagged;
    }
    
    bool Cell::isPressed() const
    {
        return mDisplayType == DisplayType::coveredPressed ||
                mDisplayType == DisplayType::questionmarkPressed;
    }
    
    bool Cell::neighbourHasMine() const
    {
        return mCountOfNeighbourMines != 0;
    }
    
    bool Cell::neighbourIsFlagged() const
    {
        return mCountOfNeigboursFlagged != 0;
    }
    
    void Cell::toggleColor(bool value)
    {
        mColorOn = value;
        update();
    }
    
    void Cell::toggleNewQuestionMarks(bool value)
    {
        mQuestionMarksOn = value;
    }
    
    void Cell::increaseCountOfFlaggedNeighbours()
    {
        ++mCountOfNeigboursFlagged;
        Q_ASSERT(mCountOfNeigboursFlagged <= 8);
    }
    
    void Cell::decreaseCountOfFlaggedNeighbours()
    {
        --mCountOfNeigboursFlagged;
        Q_ASSERT(mCountOfNeigboursFlagged >= 0);
    }
    
    void Cell::uncoverIfCoveredAndNoMine()
    {
        if (hasMine() || !isCovered()) {
            return;
        }
    
        setToUncoveredDisplayType();
        update();
    
        if(!neighbourHasMine()) {
            emit uncoverAreaWithNoMines();
        }
    }
    
    void Cell::uncoverIfNotFlagged()
    {
        if (isFLagged() || mDisplayType == DisplayType::flaggedWrong) {
            return;
        }
    
        uncover();
        update();
    
        if(!neighbourHasMine()) {
            emit uncoverAreaWithNoMines();
        }
    }
    
    void Cell::pressIfCoveredOrQuestionmark()
    { 
        if(mSingleMouseTimerLeft.isActive()) {
            mSingleMouseTimerLeft.stop();
        }
    
        if(mDisplayType == DisplayType::covered) {
            mDisplayType = DisplayType::coveredPressed;
            emit pressed();
            update();
        }
        else if(mDisplayType == DisplayType::questionmark) {
            mDisplayType = DisplayType::questionmarkPressed;
            emit pressed();
            update();
        }
    }
    
    void Cell::releaseIfCoveredOrQuestionmarkPressed()
    {
        if(mSingleMouseTimerLeft.isActive()) {
            mSingleMouseTimerLeft.stop();
        }
    
        if(mDisplayType == DisplayType::coveredPressed) {
            mDisplayType = DisplayType::covered;
            emit released();
            update();
        }
        else if(mDisplayType == DisplayType::questionmarkPressed) {
            mDisplayType = DisplayType::questionmark;
            emit released();
            update();
        }
    }
    
    void Cell::showMine()
    {
        if(hasMine()) {
            mDisplayType = DisplayType::mine;
            update();
        }
    }
    
    void Cell::setToFlaggedWrong()
    {
        mDisplayType = DisplayType::flaggedWrong;
        update();
    }
    
    void Cell::paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event)
        QPainter painter{ this };
    
        auto image = displayImage(mDisplayType);
    
        if(!mColorOn) {
            image = convertToGrayscale(image);
        }
    
        painter.drawImage(rect(), image);
    }
    
    void Cell::mark()
    {
        switch (mDisplayType) {
        case DisplayType::covered:
            mDisplayType = DisplayType::flagged;
            emit flagged();
            update();
            break;
        case DisplayType::flagged:
            if(mQuestionMarksOn) {
                mDisplayType = DisplayType::questionmark;
            }
            else {
                mDisplayType = DisplayType::covered;
            }
            emit unflagged();
            update();
            break;
        case DisplayType::questionmark:
            mDisplayType = DisplayType::covered;
            update();
            break;
        default:
            break;
        }
    }
    
    QImage Cell::displayImage(Cell::DisplayType type)
    {
        switch(type){
            case DisplayType::covered:
                return QImage{":/ressources/cell_covered.png"};
            case DisplayType::coveredPressed:
                return QImage{":/ressources/cell_covered_pressed.png"};
            case DisplayType::neigboursHave0Mines:
                return QImage{":/ressources/cell_0.png"};
            case DisplayType::neigboursHave1Mine:
                return QImage{":/ressources/cell_1.png"};
            case DisplayType::neigboursHave2Mines:
                return QImage{":/ressources/cell_2.png"};
            case DisplayType::neigboursHave3Mines:
                return QImage{":/ressources/cell_3.png"};
            case DisplayType::neigboursHave4Mines:
                return QImage{":/ressources/cell_4.png"};
            case DisplayType::neigboursHave5Mines:
                return QImage{":/ressources/cell_5.png"};
            case DisplayType::neigboursHave6Mines:
                return QImage{":/ressources/cell_6.png"};
            case DisplayType::neigboursHave7Mines:
                return QImage{":/ressources/cell_7.png"};
            case DisplayType::neigboursHave8Mines:
                return QImage{":/ressources/cell_8.png"};
            case DisplayType::questionmark:
                return QImage{":/ressources/cell_questionmark.png"};
            case DisplayType::questionmarkPressed:
                return QImage{":/ressources/cell_questionmark_pressed.png"};
            case DisplayType::flagged:
                return QImage{":/ressources/cell_flagged.png"};
            case DisplayType::mine:
                return QImage{":/ressources/cell_mine.png"};
            case DisplayType::mineExploded:
                return QImage{":/ressources/cell_mine_explode.png"};
            case DisplayType::flaggedWrong:
                return QImage{":/ressources/cell_nomine.png"};
        }
        return QImage{};
    }
    
    void Cell::uncover()
    {
        if(hasMine()) {
            uncoverMine();
        }
        else {
            setToUncoveredDisplayType();
        }
        emit uncovered();
        update();
    }
    
    void Cell::uncoverMine()
    {
        mDisplayType = DisplayType::mineExploded;
        emit hitMine();
    }
    
    void Cell::setToUncoveredDisplayType()
    {
        mDisplayType = static_cast<DisplayType>(
        static_cast<int>(
            DisplayType::neigboursHave0Mines) + mCountOfNeighbourMines);
        emit uncoveredEmptyCell();
    }
    
    
    void Cell::handleMousePressEvent(QMouseEvent *event)
    {
        if(!(event->buttons().testFlag(Qt::LeftButton) ||
             event->buttons().testFlag(Qt::RightButton))) {
            return;
        }
    
        if(event->buttons().testFlag(Qt::LeftButton)) {
            mSingleMouseTimerLeft.start();
        }
        else if (event->buttons().testFlag(Qt::RightButton)){
            mSingleMouseTimerRight.start();
        }
    
        const auto elapsedTime = mElapsedTimer.restart();
    
        if(elapsedTime >= QApplication::doubleClickInterval()) {
            return;
        }
    
        if((mSingleMouseTimerLeft.isActive() &&
            event->buttons().testFlag(Qt::RightButton)) ||
            (mSingleMouseTimerRight.isActive() &&
             event->buttons().testFlag(Qt::LeftButton))){
    
            if(!isPressed()) {
                pressIfCoveredOrQuestionmark();
                mNeighboursPressed = true;
                emit pressNeighbours();
            }
            for(QTimer* timer : { &mSingleMouseTimerRight,
                &mSingleMouseTimerLeft }) {
                timer->stop();
            }
        }
    }
    
    void Cell::handleMouseReleaseEvent(QMouseEvent *event)
    {
        if(mNeighboursPressed) {
            if(event->button() == Qt::LeftButton ||
                    event->button() == Qt::RightButton)
            {
                mNeighboursPressed = false;
    
                if(mCountOfNeigboursFlagged == mCountOfNeighbourMines) {
                    if(isPressed()) {
                        uncover();
                    }
                    emit uncoverNotFlaggedNeighbours();
                    emit uncoverAreaWithNoMines();
                }
                else {
                    if(isPressed()) {
                        releaseIfCoveredOrQuestionmarkPressed();
                    }
                    emit releaseNeighbours();
                }
            }
        }
        else if(event->button() == Qt::LeftButton) {
            uncover();
    
            if(mDisplayType == DisplayType::neigboursHave0Mines) {
                emit uncoverAreaWithNoMines();
            }
        }
    }
    
    void Cell::handleMouseMoveEventInsideLeftButton(QMouseEvent *event)
    {
        Q_UNUSED(event)
    
        if(!isPressed()) {
            pressIfCoveredOrQuestionmark();
        }
    }
    
    void Cell::handleMouseMoveEventOutsideLeftButton(QMouseEvent *event)
    {
        Q_UNUSED(event)
    
        if(mSingleMouseTimerLeft.isActive()) {
            mSingleMouseTimerLeft.stop();
        }
    
        if(isPressed()) {
            releaseIfCoveredOrQuestionmarkPressed();
        }
    }
    
    void Cell::handleMouseMoveEventInsideBothButtons(QMouseEvent *event)
    {
        handleMouseMoveEventInsideLeftButton(event);
    
        mNeighboursPressed = true;
        emit pressNeighbours();
    
    }
    
    void Cell::handleMouseMoveEventOutsideBothButtons(QMouseEvent *event)
    {
        handleMouseMoveEventOutsideLeftButton(event);
    
        if(mNeighboursPressed) {
            mNeighboursPressed = false;
            emit releaseNeighbours();
        }
    }


###cellinputhandler.h

    #ifndef CELLINPUTHANDLER_H
    #define CELLINPUTHANDLER_H
    
    #include <QObject>
    #include <QElapsedTimer>
    #include <QTimer>
    
    class Cell;
    class QMouseEvent;
    
    class CellInputHandler : public QObject
    {
        Q_OBJECT
    
    public:
        explicit CellInputHandler(QObject *parent = nullptr);
    
    protected:
        bool eventFilter(QObject *watched, QEvent *event) override;
    
    private:
        void handleMouseButtonPressEvents(QObject *watched, QEvent *event);
        void handleMouseButtonReleaseEvents(QObject *watched, QEvent *event);
        void handleMouseMoveEvents(QEvent *event);
    
        void cellMoveInsideHandle(Cell *cell, QMouseEvent *mouseEvent);
        void cellMoveOutsideHandle(Cell *cell, QMouseEvent *mouseEvent);
    
        Cell *mLastCell;
    };
    
    #endif // CELLINPUTHANDLER_H

###cellinputhandler.cpp

    #include "cellinputhandler.h"
    
    #include "cell.h"
    
    #include <QApplication>
    #include <QEvent>
    #include <QMouseEvent>
    
    #include <QDebug>
    
    CellInputHandler::CellInputHandler(QObject *parent)
        : QObject{ parent },
          mLastCell{ nullptr }
    {
    }
    
    bool CellInputHandler::eventFilter(QObject *watched, QEvent *event)
    {
        if(event->type() == QEvent::MouseButtonPress){
            handleMouseButtonPressEvents(watched, event);
            return true;
        }
        if(event->type() == QEvent::MouseButtonRelease){
            handleMouseButtonReleaseEvents(watched, event);
            return true;
        }
        if(event->type() == QEvent::MouseMove) {
            handleMouseMoveEvents(event);
            return true;
        }
        return false;
    }
    
    void CellInputHandler::handleMouseButtonPressEvents(
            QObject *watched, QEvent *event)
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        auto cell = qobject_cast<Cell *>(watched);
    
        cell->handleMousePressEvent(mouseEvent);
    
        mLastCell = cell;
    }
    
    void CellInputHandler::handleMouseButtonReleaseEvents(
            QObject *watched, QEvent *event)
    {
        Q_UNUSED(watched)
    
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        auto widget = QApplication::widgetAt(QCursor::pos());
        auto cell = qobject_cast<Cell *>(widget);
    
        if(cell) {
            cell->handleMouseReleaseEvent(mouseEvent);
            mLastCell = cell;
        }
        else if(mLastCell) {
            mLastCell->handleMouseReleaseEvent(mouseEvent);
        }
    }
    
    void CellInputHandler::handleMouseMoveEvents(QEvent *event)
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
    
        if(mouseEvent->buttons().testFlag(Qt::LeftButton)) {
            auto widget = QApplication::widgetAt(mouseEvent->globalPos());
    
            if(widget) {
                auto cell = qobject_cast<Cell *>(widget);
    
                if(mLastCell && (!cell || cell != mLastCell)) {
                    cellMoveOutsideHandle(mLastCell, mouseEvent);
                }
                if(!cell) {
                    mLastCell = nullptr;
                }
                else if(cell != mLastCell) {
                    cellMoveInsideHandle(cell, mouseEvent);
                    mLastCell = cell;
                }
            }
        }
    }
    
    void CellInputHandler::cellMoveInsideHandle(
            Cell *cell, QMouseEvent *mouseEvent)
    {
        if(mouseEvent->buttons().testFlag(Qt::RightButton)) {
            cell->handleMouseMoveEventInsideBothButtons(mouseEvent);
        }
        else {
            cell->handleMouseMoveEventInsideLeftButton(mouseEvent);
        }
    }
    
    void CellInputHandler::cellMoveOutsideHandle(
            Cell *cell, QMouseEvent *mouseEvent)
    {
        if(mouseEvent->buttons().testFlag(Qt::RightButton)) {
            cell->handleMouseMoveEventOutsideBothButtons(mouseEvent);
        }
        else {
            cell->handleMouseMoveEventOutsideLeftButton(mouseEvent);
        }
    }

###minefield.h

    #ifndef MINEFIELD_H
    #define MINEFIELD_H
    
    #include <QVector>
    #include <QWidget>
    
    #include "cell.h"
    
    #include <vector>
    
    class CellInputHandler;
    
    class Minefield : public QWidget
    {
        Q_OBJECT
    public:   
        Minefield(const QVector<Cell *> &cells, int width, int height,
                  QWidget *parent = nullptr);
    
        [[nodiscard]] int fieldWidth() const;
        [[nodiscard]] int fieldHeight() const;
        [[nodiscard]] int countOfMines() const;
        [[nodiscard]] int minesLeft() const;
    
    signals:
        void toggleColorInCells(int value);
        void toggleNewQuesionMarksInCells(int value);
    
        void uncoveredFirstCell();
        void uncoveredEmptyCell();
        void uncoveredAllSafeCells();
    
        void pressedCell();
        void releasedCell();
    
        void mineExploded();
        void minesLeftChanged(int minesLeft);  
    
    private slots:
        void flaggedCell();
        void unflaggedCell();
    
        void checkIfFirstCellIsUncovered();
        void checkIfSafeCellsUncovered();    
    
    private:   
        void connectWithCells();
        void addCellsToLayout();
    
        void showAllMines();
        void showWrongFlaggedCells();
        void disableInput();
    
        bool mFirstCellUncovered{ false };
        bool mSafeCellsUncovered{ false };
        QVector<Cell *> mCells;
        int mFieldWidth;
        int mFieldHeight;
        int mMinesLeft;
        CellInputHandler *mCellInputHandler;
    };
    
    #endif // MINEFIELD_H

###minefield.cpp

    #include "minefield.h"
    
    #include "cell.h"
    #include "cellinputhandler.h"
    #include "cellutility.h"
    
    #include <QDebug>
    #include <QGridLayout>
    
    Minefield::Minefield(const QVector<Cell *> &cells, int width, int height,
                         QWidget *parent)
        :QWidget{ parent },
          mCells{ cells },
          mFieldWidth{ width },
          mFieldHeight{ height },
          mMinesLeft{ countOfMines()},
          mCellInputHandler{ new CellInputHandler{ this } }
    {
        Q_ASSERT(mCells.size() == (mFieldWidth * mFieldHeight));
    
        connectCellsWithNeighbourCells(mCells, mFieldWidth, mFieldHeight);
    
        for(auto &cell : mCells) {
            cell->installEventFilter(mCellInputHandler);
        }
    
        connectWithCells();
        addCellsToLayout();
    }
    
    int Minefield::fieldWidth() const
    {
        return mFieldWidth;
    }
    
    int Minefield::fieldHeight() const
    {
        return mFieldHeight;
    }
    
    int Minefield::countOfMines() const
    {
        auto count = 0;
        for(const auto& cell : mCells) {
            if(cell->hasMine()) {
                ++count;
            }
        }
        return count;
    }
    
    int Minefield::minesLeft() const
    {
        return mMinesLeft;
    }
    
    void Minefield::flaggedCell()
    {
        --mMinesLeft;
        emit minesLeftChanged(mMinesLeft);
    }
    
    void Minefield::unflaggedCell()
    {
        ++mMinesLeft;
        emit minesLeftChanged(mMinesLeft);
    }
    
    
    void Minefield::checkIfFirstCellIsUncovered()
    {
        if(!mFirstCellUncovered) {
            mFirstCellUncovered = true;
    
            for(const auto &cell : mCells) {
                disconnect(cell, &Cell::uncovered,
                           this, &Minefield::checkIfFirstCellIsUncovered);
            }
    
            emit uncoveredFirstCell();
        }
    }
    
    void Minefield::checkIfSafeCellsUncovered()
    {
        if(!mSafeCellsUncovered && allSafeCellsUncovered(mCells)) {
            mSafeCellsUncovered = true;
    
            for(const auto &cell : mCells) {
                disconnect(cell, &Cell::uncovered,
                           this, &Minefield::checkIfSafeCellsUncovered);
            }
    
            disableInput();
            emit uncoveredAllSafeCells();
        }
    }
    
    void Minefield::connectWithCells()
    {
        for(const auto &cell : mCells) {
            connect(cell, &Cell::pressed,
                    this, &Minefield::pressedCell);
            connect(cell, &Cell::released,
                    this, &Minefield::releasedCell);
    
            connect(cell, &Cell::uncoveredEmptyCell,
                    this, &Minefield::uncoveredEmptyCell);
    
            connect(cell, &Cell::flagged,
                    this, &Minefield::flaggedCell);
            connect(cell, &Cell::unflagged,
                    this, &Minefield::unflaggedCell);
    
            connect(cell, &Cell::uncovered,
                    this, &Minefield::checkIfFirstCellIsUncovered);
            connect(cell, &Cell::uncovered,
                    this, &Minefield::checkIfSafeCellsUncovered);
    
            connect(this, &Minefield::toggleNewQuesionMarksInCells,
                    cell, &Cell::toggleNewQuestionMarks);
            connect(this, &Minefield::toggleColorInCells,
                    cell, &Cell::toggleColor);
    
            if(cell->hasMine()) {
                connect(cell, &Cell::hitMine,
                        [=](){
    
                    showWrongFlaggedCells();
                    showAllMines();
                    disableInput();
                    emit mineExploded();
                });
            }
        }
    }
    
    void Minefield::addCellsToLayout()
    {
        auto layout = new QGridLayout;
        layout->setSpacing(0);
        layout->setContentsMargins(0,0,0,0);
    
        for(int i = 0; i < mCells.size(); ++i) {
            auto column = static_cast<int>(i %  mFieldWidth);
            auto row = static_cast<int>(i /  mFieldWidth);
    
            layout->addWidget(mCells[i], row, column);
        }
        setLayout(layout);
    }
    
    void Minefield::showAllMines()
    {
        for(const auto cell : mCells) {
            if(cell->hasMine() && cell->isCovered()) {
                cell->showMine();
            }
        }
    }
    
    void Minefield::showWrongFlaggedCells()
    {
        for(const auto &cell : mCells) {
            if(!cell->hasMine() && cell->isFLagged()) {
                cell->setToFlaggedWrong();
            }
        }
    }
    
    void Minefield::disableInput()
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }