# [Beginner's Snake Game using SFML and C++](https://codereview.stackexchange.com/questions/221024)

Never really got past the beginner stage of programming so I'm trying to get better this Summer. I think games are a fun way to learn how to program so I decided to start with snake. I'd appreciate any feedback or learning resources you guys think might be helpful. Cheers!

**How it Works**

From the menu screen, when the user presses play, the game initializes (the snake body, food generator, and game clock are dynamically allocated). I keep these game elements in an anonymous namespace but I'm not sure if that's appropriate. After initialization, the game loop runs. Basically, Check for Collisions -> Check if Eating -> Update Snake Nodes -> Render and Display. 

**Game.cpp**

    #include "Game.h"
    
    namespace
    {
    	SnakeBody *snakebody;
    	FoodGenerator *foodgenerator;
    	sf::Clock *gameclock;
    }
    
    void Game::Start()
    {
    	if (mGameState != UNINITIALIZED)
    		return;
    
    	mMainWindow.create(sf::VideoMode(windowparameters::RESOLUTION_X, windowparameters::RESOLUTION_Y, windowparameters::COLOR_DEPTH), "Snake!");
    	mGameState = SHOWING_MENU;
    
    	while (mGameState != EXITING)
    		GameLoop();
    
    	mMainWindow.close();
    }
    
    void Game::ShowMenuScreen()
    {
    	MainMenu menuScreen;
    	MainMenu::MenuResult result = menuScreen.Show(mMainWindow);
    
    	switch (result)
    	{
    	case MainMenu::Exit:
    		mGameState = EXITING;
    		break;
    
    	case MainMenu::Play:
    		mGameState = RUNNING;
    		break;
    	}
    }
    
    Game::GameState Game::WaitForEnterOrExit()
    {
    	GameState nextstate = GAMEOVER;
    	sf::Event currentevent;
    
    	while (nextstate != EXITING && nextstate != RUNNING)
    	{
    		while (mMainWindow.pollEvent(currentevent))
    		{
    			if (currentevent.type == sf::Event::EventType::KeyPressed && 
    				sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
    			{
    				nextstate = RUNNING;
    			}
    			else if (currentevent.type == sf::Event::EventType::Closed)
    			{
    				nextstate = EXITING;
    			}
    		}
    	}
    
    	return nextstate;
    }
    
    void Game::InitializeGameElements()
    {
    	snakebody = new SnakeBody();
    	foodgenerator = new FoodGenerator(windowparameters::RESOLUTION_X, windowparameters::RESOLUTION_Y, windowparameters::UNIT_SPACING);
    	gameclock = new sf::Clock();
    }
    
    void Game::CleanupGameElements()
    {
    	delete(gameclock);
    	delete(snakebody);
    	delete(foodgenerator);
    }
    
    void Game::HandleEvents()
    {
    	sf::Event currentevent;
    
    	while (mMainWindow.pollEvent(currentevent))
    	{
    		if (currentevent.type == sf::Event::EventType::KeyPressed)
    		{
    			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    			{
    				snakebody->RedirectHead(SnakeBody::LEFT);
    			}
    			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    			{
    				snakebody->RedirectHead(SnakeBody::RIGHT);
    			}
    			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    			{
    				snakebody->RedirectHead(SnakeBody::UP);
    			}
    			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    			{
    				snakebody->RedirectHead(SnakeBody::DOWN);
    			}
    			break;
    		}
    		else if (currentevent.type == sf::Event::EventType::Closed)
    		{
    			mGameState = EXITING;
    			mMainWindow.close();
    		}
    	}
    }
    
    void Game::GameTick()
    {
    	// tick scene
    	if (gameclock->getElapsedTime().asMilliseconds() >= windowparameters::TIC_RATE_IN_MS)
    	{
    		// Check Collision with body
    		if (snakebody->CheckCollision())
    			mGameState = GAMEOVER;
    		
    		else if (snakebody->CheckEating(foodgenerator->mGraphic))
    		{
    			snakebody->IncrementSegments();
    			foodgenerator->mUneaten = false;
    
    			std::cout << "SCORE = " << snakebody->mNumSegments << std::endl;
    		}
    
    		// update snake
    		snakebody->UpdateSegments(0, windowparameters::RESOLUTION_X, 0, windowparameters::RESOLUTION_Y);
    
    		// update food
    		if (!foodgenerator->mUneaten)
    			foodgenerator->Generate(snakebody);
    
    		// reset screen, render, display
    		mMainWindow.clear(sf::Color(230, 230, 230));
    
    		mMainWindow.draw(foodgenerator->mGraphic);
    		snakebody->DrawSegments(mMainWindow);
    
    		mMainWindow.display();
    		gameclock->restart();
    	}
    }
    
    void Game::GameLoop()
    {
    	while (true)
    	{
    		switch (mGameState)
    		{
    		case SHOWING_MENU:
    			ShowMenuScreen();
    			break;
    
    		case GAMEOVER:
    			mGameState = WaitForEnterOrExit();
    			break;
    		
    		case RUNNING:
    
    			InitializeGameElements();
    			
    			// run game loop
    			while (mMainWindow.isOpen() && mGameState == RUNNING)
    			{
    				HandleEvents();
    				GameTick();
    			}
    
    			CleanupGameElements();
    			break;
    
    		case EXITING:
    			mMainWindow.close();
    			break;
    
    		default:
    			mMainWindow.close();
    			break;
    		}
    	}
    }
    
    // Because Game is a static class, the member variables need to be instantiated MANUALLY
    Game::GameState Game::mGameState = Game::UNINITIALIZED;
    sf::RenderWindow Game::mMainWindow;


**Game.h**


    #pragma once
    #include <cstdint>
    #include <iostream>
    
    #include "FoodGenerator.h"
    #include "MainMenu.h"
    #include "SFML/Window.hpp"
    #include "SFML/Graphics.hpp"
    #include "SnakeBody.h"
    
    namespace windowparameters
    {
    	const uint16_t RESOLUTION_X = 1024;
    	const uint16_t RESOLUTION_Y = 768;
    	const uint8_t COLOR_DEPTH = 32;
    	const uint16_t TIC_RATE_IN_MS = 60;
    	const uint8_t UNIT_SPACING = 32;
    }
    
    class Game
    {
    public:
    	static void Start();
    
    private:
    	enum GameState { UNINITIALIZED, SHOWING_MENU, RUNNING, EXITING, GAMEOVER };
    
    	static void GameLoop();
    	static void ShowMenuScreen();
    	static void InitializeGameElements();
    	static void CleanupGameElements();
    	static void HandleEvents();
    	static void GameTick();
    	static GameState WaitForEnterOrExit();
    
    	static GameState mGameState;
    	static sf::RenderWindow mMainWindow;
    };



**FoodGenerator.h**


    #pragma once
    
    #include <random>
    
    #include "Coordinate.h"
    #include "SnakeBody.h"
    
    class FoodGenerator
    {
    public:
    	FoodGenerator::FoodGenerator(int xmax, int ymax, int spacing);
    	Coordinate Generate(SnakeBody *snakeBody);
    	bool mUneaten;
    	int mXMax;
    	int mYMax;
    	int mSpacing;
    	Coordinate mCurrentLocation;
    	sf::RectangleShape mGraphic;
    
    private:
    	std::uniform_int_distribution<int> uniX;
    	std::uniform_int_distribution<int> uniY;
    	std::random_device rd;
    	std::mt19937 rng;
    };


**FoodGenerator.cpp**

    #include "FoodGenerator.h"
    
    FoodGenerator::FoodGenerator(int xmax, int ymax, int spacing)
    {
    	rng = std::mt19937(rd());    // random-number engine used (Mersenne-Twister in this case)
    	uniX = std::uniform_int_distribution<int>(1, xmax/spacing - 1); // guaranteed unbiased
    	uniY = std::uniform_int_distribution<int>(1, ymax/spacing - 1); // guaranteed unbiased
    
    	mGraphic = sf::RectangleShape(sf::Vector2f(spacing, spacing));
    	mGraphic.setFillColor(sf::Color(0, 0, 128));
    	mGraphic.setOrigin(0, 0);
    
    	mUneaten = false;
    	mXMax = xmax;
    	mYMax = ymax;
    	mSpacing = spacing;
    }
    
    Coordinate FoodGenerator::Generate(SnakeBody *snakeBody)
    {
    	bool freePosFound = false;
    	int xPos, yPos;
    
    	std::list<SnakeBody::SnakeSegment>::iterator it, head, end;
    	it = snakeBody->mSegments.begin();
    	head = snakeBody->mSegments.begin();
    	end = snakeBody->mSegments.end();
    
    	while (!freePosFound)
    	{
    		xPos = uniX(rng);
    		yPos = uniY(rng);
    
    		mGraphic.setPosition(xPos*mSpacing, yPos*mSpacing);
    
    		while (it != end)
    		{
    			if (it->mGraphic.getGlobalBounds().intersects(mGraphic.getGlobalBounds()))
    			{
    				it = head;
    				break;
    			}
    
    			it++;
    		}
    
    		if (it == end)
    			freePosFound = true;
    	}
    	mUneaten = true;
    	return Coordinate(xPos, yPos);
    }



**MainMenu.h**

    #pragma once
    
    #include <list>
    
    #include "SFML/Graphics.hpp"
    
    class MainMenu
    {
    public:
    	enum MenuResult {Nothing, Exit, Play};
    
    	struct MenuItem
    	{
    		MenuResult action;
    		sf::Rect<int> rect;
    	};
    
    	MenuResult Show(sf::RenderWindow& window);
    
    private:
    	MenuResult GetMenuResponse(sf::RenderWindow& window);
    	MenuResult HandleClick(int x, int y);
    	std::list<MenuItem> mMenuItems;
    };



**MainMenu.cpp** 

    #include "MainMenu.h"
    
    MainMenu::MenuResult MainMenu::Show(sf::RenderWindow& window)
    {
    	sf::Texture image;
    	image.loadFromFile("C:/Users/Carter/Pictures/snake_menu.jpg");
    	sf::Sprite sprite(image);
    
    	MenuItem playButton;
    	playButton.rect.left = 200;
    	playButton.rect.top = 525;
    	playButton.rect.width = 600;
    	playButton.rect.height = 100;
    	playButton.action = Play;
    
    	MenuItem exitButton;
    	exitButton.rect.left = 200;
    	exitButton.rect.top = 630;
    	exitButton.rect.width = 600;
    	exitButton.rect.height = 100;
    	exitButton.action = Exit;
    
    	mMenuItems.push_back(playButton);
    	mMenuItems.push_back(exitButton);
    
    	window.draw(sprite);
    	window.display();
    
    	return GetMenuResponse(window);
    }
    
    MainMenu::MenuResult MainMenu::HandleClick(int x, int y)
    {
    	std::list<MenuItem>::iterator it;
    
    	for (it = mMenuItems.begin(); it != mMenuItems.end(); it++)
    	{
    		sf::Rect<int> menuItemRect = (*it).rect;
    
    		if((x > menuItemRect.left) &&
    			(x < (menuItemRect.left + menuItemRect.width)) &&
    			(y > menuItemRect.top) &&
    			(y < (menuItemRect.top + menuItemRect.height)))
    		{
    			return (*it).action;
    		}
    	}
    	return Nothing;
    }
    
    MainMenu::MenuResult MainMenu::GetMenuResponse(sf::RenderWindow& window)
    {
    	sf::Event menuEvent;
    
    	while (true)
    	{
    		while (window.pollEvent(menuEvent))
    		{
    			if (menuEvent.type == sf::Event::EventType::MouseButtonPressed)
    				return HandleClick(menuEvent.mouseButton.x, menuEvent.mouseButton.y);
    			
    			if (menuEvent.type == sf::Event::EventType::Closed)
    				return Exit;
    		}
    	}
    }


**SnakeBody.h**

    #pragma once
    
    #include <cstdint>
    #include <list>
    
    #include "Coordinate.h"
    #include "SFML/Graphics.hpp"
    
    
    class SnakeBody
    {
    public:
    	SnakeBody();
    
    	enum SnakeDirection { LEFT, RIGHT, UP, DOWN };
    
    	class SnakeSegment
    	{
    	public:
    		SnakeSegment(int x, int y, SnakeDirection dir);
    		void UpdatePosition();
    		bool CheckBounds(int xmin, int xmax, int ymin, int ymax);
    		Coordinate GetPosition();
    		void SetPosition(int x, int y);
    		SnakeDirection GetDirection();
    		void SetDirection(SnakeDirection dir);
    
    		sf::RectangleShape mGraphic;
    
    	private:
    		Coordinate mPosition;
    		SnakeDirection mDirection;
    	};
    
    	void UpdateSegments(int xmin, int xmax, int ymin, int ymax);
    	void DrawSegments(sf::RenderWindow &window);
    	void RedirectHead(SnakeDirection newDir);
    	void IncrementSegments();
    	bool CheckCollision();
    	bool CheckEating(sf::RectangleShape foodGraphic);
    	
    	int mNumSegments;
    	std::list<SnakeSegment> mSegments;
    };



**SnakeBody.cpp**

    #include "SnakeBody.h"
    
    namespace
    {
    	const uint8_t SNAKE_MOVE_PER_TICK = 32;
    	const uint8_t BODY_DIM = 32;
    }
    
    SnakeBody::SnakeSegment::SnakeSegment(int x, int y, SnakeBody::SnakeDirection dir)
    {
    	SetPosition(x, y);
    	SetDirection(dir);
    
    	mGraphic = sf::RectangleShape(sf::Vector2f(BODY_DIM, BODY_DIM));
    	mGraphic.setFillColor(sf::Color(34, 139, 34));
    	mGraphic.setOrigin(BODY_DIM / 2, BODY_DIM / 2);
    	mGraphic.setPosition(sf::Vector2f(x, y));
    }
    
    Coordinate SnakeBody::SnakeSegment::GetPosition()
    {
    	return mPosition;
    }
    
    void SnakeBody::SnakeSegment::SetPosition(int x, int y)
    {
    	mPosition.mXCoord = x;
    	mPosition.mYCoord = y;
    	mGraphic.setPosition(sf::Vector2f(x, y));
    }
    
    SnakeBody::SnakeDirection SnakeBody::SnakeSegment::GetDirection()
    {
    	return mDirection;
    }
    
    void SnakeBody::SnakeSegment::SetDirection(SnakeBody::SnakeDirection dir)
    {
    	// prevent 180 degree turns about the head
    	switch (dir)
    	{
    	case LEFT:
    		if (mDirection == RIGHT)
    			return;
    		break;
    
    	case RIGHT:
    		if (mDirection == LEFT)
    			return;
    		break;
    
    	case UP:
    		if (mDirection == DOWN)
    			return;
    		break;
    
    	case DOWN:
    		if (mDirection == UP)
    			return;
    		break;
    	}
    
    	SnakeSegment::mDirection = dir;
    }
    
    bool SnakeBody::SnakeSegment::CheckBounds(int xmin, int xmax, int ymin, int ymax)
    {
    	bool wrapped = false;
    	int xrange = xmax - xmin;
    	int yrange = ymax - ymin;
    
    	// check bounds and wrap
    	if (mPosition.mXCoord < xmin)
    	{
    		mPosition.mXCoord += xrange;
    		wrapped = true;
    	}
    	else if (mPosition.mXCoord > xmax)
    	{
    		mPosition.mXCoord %= xrange;
    		wrapped = true;
    	}
    	else if (mPosition.mYCoord < ymin)
    	{
    		mPosition.mYCoord += yrange;
    		wrapped = true;
    	}
    
    	else if (mPosition.mYCoord > ymax)
    	{
    		mPosition.mYCoord %= yrange;
    		wrapped = true;
    	}
    
    	if(wrapped)
    		mGraphic.setPosition(mPosition.mXCoord, mPosition.mYCoord);
    
    	return wrapped;
    }
    
    
    void SnakeBody::SnakeSegment::UpdatePosition()
    {
    	// check direction and increment
    	switch (mDirection)
    	{
    	case LEFT:
    		mPosition.IncrementX(-SNAKE_MOVE_PER_TICK);
    		break;
    	case RIGHT:
    		mPosition.IncrementX(SNAKE_MOVE_PER_TICK);
    		break;
    	case UP:
    		mPosition.IncrementY(-SNAKE_MOVE_PER_TICK);
    		break;
    	case DOWN:
    		mPosition.IncrementY(SNAKE_MOVE_PER_TICK);
    		break;
    	}
    
    	mGraphic.setPosition(sf::Vector2f(mPosition.mXCoord, mPosition.mYCoord));
    }
    
    SnakeBody::SnakeBody()
    {
    	SnakeBody::SnakeSegment headSegment(BODY_DIM/2, BODY_DIM/2, RIGHT);
    	//SnakeBody::SnakeSegment testSegment(100 - BODY_DIM, 100, RIGHT);
    	mNumSegments = 1;
    	mSegments.push_back(headSegment);
    	//_segments.push_back(testSegment);
    }
    
    void SnakeBody::UpdateSegments(int xmin, int xmax, int ymin, int ymax)
    {
    	// update segments starting at tail
    	std::list<SnakeSegment>::iterator front, it, next, end;
    	it = --mSegments.end();
    	end = mSegments.end();
    
    	if (mNumSegments > 1)
    		next = --(--mSegments.end());
    	else
    		next = end;
    
    	front = mSegments.begin();
    
    	for(int i=0; i < mNumSegments; i++)
    	{
    		// increment position
    		it->UpdatePosition();
    		it->CheckBounds(xmin, xmax, ymin, ymax);
    
    		// update direction for non-head nodes
    		if ((it != front) && (it->GetDirection() != next->GetDirection())){
    			it->SetDirection(next->GetDirection());
    		}
    
    		if ((next != front) && next != end)
    			next--;
    
    		if (it != front)
    			it--;
    	}
    }
    
    void SnakeBody::DrawSegments(sf::RenderWindow &window)
    {
    	std::list<SnakeSegment>::iterator it = mSegments.begin();
    	std::list<SnakeSegment>::iterator end = mSegments.end();
    
    	while (it != end)
    	{
    		window.draw(it->mGraphic);
    		it++;
    	}
    	
    }
    
    void SnakeBody::RedirectHead(SnakeBody::SnakeDirection newDir)
    {
    	std::list<SnakeSegment>::iterator head = mSegments.begin();
    	head->SetDirection(newDir);
    }
    
    void SnakeBody::IncrementSegments()
    {
    	// find location of last node
    	std::list<SnakeSegment>::iterator tail = --mSegments.end();
    
    	// spawn at offset location
    	int newX, newY;
    	newX = (tail->GetPosition()).mXCoord;
    	newY = (tail->GetPosition()).mYCoord;
    
    	switch (tail->GetDirection())
    	{
    	case LEFT:
    		newX += BODY_DIM;
    		break;
    	case RIGHT: 
    		newX -= BODY_DIM;
    		break;
    	case UP:
    		newY += BODY_DIM;
    		break;
    	case DOWN:
    		newY -= BODY_DIM;
    		break;
    	}
    	SnakeSegment newSegment(newX, newY, tail->GetDirection());
    	mSegments.push_back(newSegment);
    	mNumSegments++;
    }
    
    bool SnakeBody::CheckCollision()
    {
    	sf::RectangleShape headRect = (mSegments.begin())->mGraphic;
    	std::list<SnakeSegment>::iterator it = ++mSegments.begin();
    
    	for (int i = 1; i < mNumSegments; i++, it++)
    	{
    		if (headRect.getGlobalBounds().intersects(it->mGraphic.getGlobalBounds()))
    			return true;
    	}
    	return false;
    }
    
    bool SnakeBody::CheckEating(sf::RectangleShape foodGraphic)
    {
    	std::list<SnakeSegment>::iterator head = mSegments.begin();
    
    	return head->mGraphic.getGlobalBounds().intersects(foodGraphic.getGlobalBounds());
    }


**main.cpp**

    #include "Game.h"
    
    int main()
    {
    	Game::Start();
    
    	return 0;
    }

