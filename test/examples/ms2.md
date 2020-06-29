# [Simple Minesweeper using OpenGL / GLUT](https://codereview.stackexchange.com/questions/158957)
### tags: ['c++', 'opengl', 'minesweeper']

I'm trying to make minesweeper similar to windows 3.1 minesweeper by using OpenGL / GLUT. The game still in early stage but playable. I would like to know, how can I improve it.

EDIT 1:
i added additional draw functions and fixed drawing order when player step on mine tile.

here image for latest update:

[![enter image description here][1]][1]

Edit 2:
i have added timer class for animating when player is win or lose to demo and fixed index coordinate from mouse input. also, i added option to restart game at any time by click on game icon "smiley face"

Edit 3):
added drawing functions for primitive shapes (rect, circle) to avoid duplicated.
added Color for readability.
finally, anti-alias is working perfectly under GLUT context. 

Edit 4):
removed the old window time with the c++11 std::chrono 




    #include <cmath>
    #include <iostream>
    #include <random>
    #include <chrono>
    
    #include <gl/glut.h>
    
    
    enum { MINE = 9 };
    enum { TILE_SIZE = 20 };
    enum { MARGIN = 40 };
    enum { PADDING = 10 };
    enum { BOARD_SIZE = 9 };
    enum { MINE_COUNT = 10 };
    
    enum Color {
    	RED,
    	DARKRED,
    	BLUE,
    	DARKBLUE,
    	GREEN,
    	DARKGREEN,
    	CYAN,
    	DARKCYAN,
    	YELLOW,
    	DARKYELLOW,
    	WHITE,
    	MAGENTA,
    	BLACK,
    	DARKGRAY,
    	LIGHTGRAY,
    	ULTRALIGHTGRAY
    };
    
    static const struct
    {
    	float r, g, b;
    } colors[] =
    {
    	{ 1, 0, 0 },// red
    	{ 0.5f, 0, 0 },// dark red
    
    	{ 0, 0, 1 }, // blue
    	{ 0, 0, 0.5f }, // dark blue
    
    	{ 0, 1, 0 }, // green
    	{ 0, 0.5f, 0 }, // dark green
    
    	{ 0, 1, 1 }, // cyan
    	{ 0, 0.5f, 0.5f }, // dark  cyan
    
    	{ 1, 1, 0 },//yellow
    	{ 0.5f, 0.5f, 0 },//dark yellow
    
    	{ 1, 1, 1 },// White
    	{ 1, 0, 1 }, // magenta
    
    	{ 0, 0, 0 }, // black
    	{ 0.25, 0.25, 0.25 }, // dark gray
    	{ 0.5, 0.5, 0.5 }, // light gray
    	{ 0.75, 0.75, 0.75 }, // ultra-light gray
    
    };
    
    class  Clock
    {
    	typedef std::chrono::time_point<std::chrono::system_clock> time_point;
    public:
    	Clock()
    		: m_startTime(getCurrentTime())
    		, m_lastTime()
    	{
    	}
    
    	double getElapsedTime() const
    	{
    		std::chrono::duration<double> elapsed = getCurrentTime() - m_startTime;
    		return elapsed.count();
    	}
    
    	double restart()
    	{
    		time_point now = getCurrentTime();
    		std::chrono::duration<double> elapsed = now - m_startTime;
    		m_startTime = now;
    
    		return elapsed.count();
    	}
    
    	static time_point getCurrentTime()
    	{
    		return std::chrono::system_clock::now();
    	}
    
    private:
    	time_point m_startTime;
    	time_point m_lastTime;
    
    }game_clock;
    
    struct cell
    {
    	int type;
    	bool flag;
    	bool open;
    };
    
    cell board[BOARD_SIZE*BOARD_SIZE];
    int death;
    int width;
    int height;
    bool clicked;
    int num_opened;
    
    
    int rand_int(int low, int high)
    {
    	static std::default_random_engine re{ std::random_device{}() };
    	using Dist = std::uniform_int_distribution<int>;
    	static Dist uid{};
    	return uid(re, Dist::param_type{ low,high });
    }
    
    void drawRect(int x, int y, float width, float height, const Color& color = LIGHTGRAY, bool outline = true)
    {
    	glColor3f(colors[color].r, colors[color].g, colors[color].b);
    	glBegin(outline ? GL_LINE_STRIP : GL_TRIANGLE_FAN);
    	{
    		glVertex2i(x + 0 * width, y + 0 * height);
    		glVertex2i(x + 1 * width, y + 0 * height);
    		glVertex2i(x + 1 * width, y + 1 * height);
    		glVertex2i(x + 0 * width, y + 1 * height);
    	}
    	glEnd();
    }
    
    void drawCircle(int cx, int cy, float radius, const Color& color = LIGHTGRAY, bool outline = true)
    {
    	glColor3f(colors[color].r, colors[color].g, colors[color].b);
    	glBegin(outline ? GL_LINE_LOOP : GL_TRIANGLE_FAN);
    	for (int i = 0; i <= 32; i++) {
    		float angle = 2 * 3.14159 * i / 32.0f;
    		float x = radius * cosf(angle);
    		float y = radius * sinf(angle);
    		glVertex2f(x+cx, y+cy);
    	}
    	glEnd();
    }
    
    void drawFlag(int x, int y)
    {
    	glColor3f(colors[BLACK].r, colors[BLACK].g, colors[BLACK].b);
    	x = (x*TILE_SIZE) + PADDING + 6;
    	y = (y*TILE_SIZE) + PADDING + 3;
    
    	//platform
    	glBegin(GL_POLYGON);
    	{
    		glVertex2i(x + 0, y + 2);
    		glVertex2i(x + 9, y + 2);
    		glVertex2i(x + 9, y + 3);
    		glVertex2i(x + 7, y + 3);
    		glVertex2i(x + 7, y + 4);
    		glVertex2i(x + 3, y + 4);
    		glVertex2i(x + 3, y + 3);
    		glVertex2i(x + 0, y + 3);
    	}
    	glEnd();
    
    	//mast
    	glBegin(GL_LINES);
    	{
    		glVertex2i(x + 4, y + 4);
    		glVertex2i(x + 4, y + 7);
    	}
    	glEnd();
    
    	//flag
    	glColor3f(colors[RED].r, colors[RED].g, colors[RED].b);
    	glBegin(GL_TRIANGLES);
    	{
    		glVertex2i(x + 5, y + 7);
    		glVertex2i(x + 5, y + 12);
    		glVertex2i(x + 0, y + 9);
    	}
    	glEnd();
    }
    
    void drawMine(int x, int y, bool dead)
    {
    	if (dead)
    	{
    		drawRect(x*TILE_SIZE + PADDING, y*TILE_SIZE + PADDING, TILE_SIZE, TILE_SIZE, RED, false);
    	}
    
    
    	x = (x*TILE_SIZE) + PADDING + 4;
    	y = (y*TILE_SIZE) + PADDING + 4;
    
    	//spikes
    	glColor3f(colors[BLACK].r, colors[BLACK].g, colors[BLACK].b);
    	glBegin(GL_LINES);
    	{
    		glVertex2i(x + 5, y - 1);
    		glVertex2i(x + 5, y + 12);
    
    		glVertex2i(x - 1, y + 5);
    		glVertex2i(x + 12, y + 5);
    
    		glVertex2i(x + 1, y + 1);
    		glVertex2i(x + 10, y + 10);
    
    		glVertex2i(x + 1, y + 10);
    		glVertex2i(x + 10, y + 1);
    	}
    	glEnd();
    
    	//ball
    	glBegin(GL_POLYGON);
    	{
    		glVertex2i(x + 3, y + 1);
    		glVertex2i(x + 1, y + 4);
    		glVertex2i(x + 1, y + 7);
    		glVertex2i(x + 3, y + 10);
    		glVertex2i(x + 8, y + 10);
    		glVertex2i(x + 10, y + 7);
    		glVertex2i(x + 10, y + 4);
    		glVertex2i(x + 8, y + 1);
    	}
    	glEnd();
    
    	//shine
    	drawRect(x+3, y+5, 2, 2, WHITE, false);
    }
    
    void drawNum(int x, int y, int v)
    {
    	switch (v)
    	{
    	case 1:
    		glColor3f(colors[BLUE].r, colors[BLUE].g, colors[BLUE].b);
    		break;
    	case 2:
    		glColor3f(colors[GREEN].r, colors[GREEN].g, colors[GREEN].b);
    		break;
    	case 3:
    		glColor3f(colors[RED].r, colors[RED].g, colors[RED].b);
    		break;
    	case 4:
    		glColor3f(colors[DARKBLUE].r, colors[DARKBLUE].g, colors[DARKBLUE].b);
    		break;
    	case 5:
    		glColor3f(colors[DARKRED].r, colors[DARKRED].g, colors[DARKRED].b);
    		break;
    	case 6:
    		glColor3f(colors[DARKYELLOW].r, colors[DARKYELLOW].g, colors[DARKYELLOW].b);
    		break;
    	case 7:
    		glColor3f(colors[CYAN].r, colors[CYAN].g, colors[CYAN].b);
    		break;
    	case 8:
    		glColor3f(colors[DARKCYAN].r, colors[DARKCYAN].g, colors[DARKCYAN].b);
    		break;
    	}
    	glRasterPos2i((x + 0)*TILE_SIZE + PADDING + 6, (y + 0)*TILE_SIZE + PADDING + 5);
    	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, '0' + v);
    }
    
    
    void drawFrame(float x, float y, float width, float height, bool doubleFrame = true)
    {
    
    	glColor3f(colors[WHITE].r, colors[WHITE].g, colors[WHITE].b);
    	glBegin(GL_LINE_LOOP);
    	{
    		glVertex2f((x + 0) + 0 * width, (y - 0) + 0 * height);
    		glVertex2f((x - 0) + 0 * width, (y - 1) + 1 * height);
    		glVertex2f((x - 1) + 1 * width, (y - 1) + 1 * height);
    		glVertex2f((x - 2) + 1 * width, (y - 2) + 1 * height);
    		glVertex2f((x + 1) + 0 * width, (y - 2) + 1 * height);
    		glVertex2f((x + 1) + 0 * width, (y + 1) + 0 * height);
    	}
    	glEnd();
    
    	glColor3f(colors[LIGHTGRAY].r, colors[LIGHTGRAY].g, colors[LIGHTGRAY].b);
    	glBegin(GL_LINE_LOOP);
    	{
    		glVertex2f((x - 2) + 1 * width, (y - 2) + 1 * height);
    		glVertex2f((x - 2) + 1 * width, (y + 1) + 0 * height);
    		glVertex2f((x + 1) + 0 * width, (y + 1) + 0 * height);
    		glVertex2f((x - 0) + 0 * width, (y - 0) + 0 * height);
    		glVertex2f((x - 1) + 1 * width, (y - 0) + 0 * height);
    		glVertex2f((x - 1) + 1 * width, (y - 1) + 1 * height);
    	}
    	glEnd();
    
    	if (!doubleFrame) return;
    
    	width = width - 2 * PADDING;
    	height = height - 2 * PADDING;
    
    
    	glBegin(GL_LINE_LOOP);
    	{
    		glVertex2f((x - 0 + PADDING) + 0 * width, (y + PADDING - 0) + 0 * height);
    		glVertex2f((x - 0 + PADDING) + 0 * width, (y + PADDING - 1) + 1 * height);
    		glVertex2f((x - 1 + PADDING) + 1 * width, (y + PADDING - 1) + 1 * height);
    		glVertex2f((x - 2 + PADDING) + 1 * width, (y + PADDING - 2) + 1 * height);
    		glVertex2f((x + 1 + PADDING) + 0 * width, (y + PADDING - 2) + 1 * height);
    		glVertex2f((x + 1 + PADDING) + 0 * width, (y + PADDING + 1) + 0 * height);
    	}
    	glEnd();
    	glColor3f(colors[WHITE].r, colors[WHITE].g, colors[WHITE].b);
    
    	glBegin(GL_LINE_LOOP);
    	{
    		glVertex2i((x + PADDING - 2) + 1 * width, (y + PADDING - 2) + 1 * height);
    		glVertex2i((x + PADDING - 2) + 1 * width, (y + PADDING + 1) + 0 * height);
    		glVertex2i((x + PADDING + 1) + 0 * width, (y + PADDING + 1) + 0 * height);
    		glVertex2i((x + PADDING - 0) + 0 * width, (y + PADDING - 0) + 0 * height);
    		glVertex2i((x + PADDING - 1) + 1 * width, (y + PADDING - 0) + 0 * height);
    		glVertex2i((x + PADDING - 1) + 1 * width, (y + PADDING - 1) + 1 * height);
    	}
    	glEnd();
    }
    
    void drawClosedDim(int x, int y)
    {
    	drawFrame(x *TILE_SIZE + PADDING, y*TILE_SIZE + PADDING, TILE_SIZE, TILE_SIZE, false);
    }
    
    void drawOpenDim(int x, int y)
    {
    	drawRect(x*TILE_SIZE + PADDING, y*TILE_SIZE + PADDING, TILE_SIZE, TILE_SIZE);
    }
    
    void drawUpperFrame(int x = 0, int y = 0)
    {
    	static const float upper_frame_outter_width = width;
    	static const float upper_frame_outter_height = 2 * MARGIN;
    	static const float offset = height - upper_frame_outter_height;
    
    	drawFrame(0, offset, upper_frame_outter_width, upper_frame_outter_height);
    }
    
    void drawLowerFrame(int x = 0, int y = 0)
    {
    	static const float lower_frame_outter_size = width;
    	drawFrame(0, 0, lower_frame_outter_size, lower_frame_outter_size);
    }
    
    void drawIcon(int x = 0, int y = 0)
    {
    	static const float icon_size = 2 * TILE_SIZE;
    	if (clicked)
    	{
    		int x = 0, y = 0;
    		static const float cx = (width - icon_size) / 2.0f;
    		static const float cy = (height - MARGIN) - icon_size / 2.0f;
    		drawRect(cx, cy, 2 * TILE_SIZE, 2 * TILE_SIZE, ULTRALIGHTGRAY, false);
    
    		if (game_clock.getElapsedTime() > 0.25) {
    			clicked = false;
    			game_clock.restart();
    		}
    	}
    
    	drawFrame((width - icon_size) / 2.0f, (height - MARGIN) - icon_size / 2.0f, icon_size, icon_size, false);
    
    	static const float cx = width / 2.0f;
    	static const float cy = (height - MARGIN);
    
    	// face
    	drawCircle(x + cx, y + cy, TILE_SIZE*0.707f, YELLOW, false);
    	drawCircle(x + cx, y + cy, TILE_SIZE*0.707f, DARKGRAY);
    
    	// eyes
    	glBegin(GL_POINTS);
    	glVertex2f(-4.707 + cx, 1.707 + cy);
    	glVertex2f(4.707 + cx, 1.707 + cy);
    	glEnd();
    
    	// mouth
    	glBegin(GL_LINES);
    	{
    		glVertex2f(-3.707 + cx, -8.707 + cy);
    		glVertex2f(3.707 + cx, -8.707 + cy);
    	}
    	glEnd();
    }
    
    int index(int x, int y)
    {
    	return x + (y*BOARD_SIZE);
    }
    
    bool isOpen(int x, int y)
    {
    	return board[index(x, y)].open;
    }
    
    
    int getType(int x, int y)
    {
    	return board[index(x, y)].type;
    }
    
    void setType(int x, int y, int v)
    {
    	board[index(x, y)].type = v;
    }
    
    bool isMine(int x, int y)
    {
    	if (x < 0 || y < 0 || x > BOARD_SIZE - 1 || y > BOARD_SIZE - 1)
    		return false;
    
    	if (getType(x, y) == MINE)
    		return true;
    	return false;
    }
    
    int calcMine(int x, int y)
    {
    	return isMine(x - 1, y - 1)
    		+ isMine(x, y - 1)
    		+ isMine(x + 1, y - 1)
    		+ isMine(x - 1, y)
    		+ isMine(x + 1, y)
    		+ isMine(x - 1, y + 1)
    		+ isMine(x, y + 1)
    		+ isMine(x + 1, y + 1);
    }
    
    bool isFlag(int x, int y)
    {
    	return board[index(x, y)].flag;
    }
    
    bool gameOver()
    {
    	return death != -1;
    }
    
    bool isDead(int x, int y)
    {
    	return death == index(x, y);
    }
    
    bool hasWon()
    {
    	return num_opened == MINE_COUNT;
    }
    
    void openMines(bool open = true)
    {
    	for (int y = 0; y < BOARD_SIZE; y++) {
    		for (int x = 0; x < BOARD_SIZE; x++) {
    			if (isMine(x, y))
    				board[index(x, y)].open = open;
    		}
    	}
    }
    
    void openCell(int x, int y)
    {
    	if (x < 0 || y < 0 || y > BOARD_SIZE - 1 || x > BOARD_SIZE - 1)
    		return;
    	if (isOpen(x, y))
    		return;
    	num_opened--;
    	board[index(x, y)].open = true;
    	if (isMine(x, y))
    	{
    		death = index(x, y);
    		openMines();
    		return;
    	}
    
    	if (getType(x, y) == 0)
    	{
    		openCell(x - 1, y + 1);
    		openCell(x, y + 1);
    		openCell(x + 1, y + 1);
    		openCell(x - 1, y);
    		openCell(x + 1, y);
    		openCell(x - 1, y - 1);
    		openCell(x, y - 1);
    		openCell(x + 1, y - 1);
    	}
    }
    
    void toggleFlag(int x, int y)
    {
    	board[index(x, y)].flag = !isFlag(x, y);
    }
    
    void drawOpen(int x, int y, int n, bool dead)
    {
    	switch (n) {
    	case 0:
    		drawOpenDim(x, y);
    		break;
    	case 9:
    		if (!dead) {
    			drawOpenDim(x, y);
    		}
    		drawMine(x, y, dead);
    		break;
    	default:
    		drawOpenDim(x, y);
    		drawNum(x, y, n);
    	}
    }
    
    void drawClosed(int x, int y)
    {
    	drawClosedDim(x, y);
    	if (isFlag(x, y))
    		drawFlag(x, y);
    }
    
    void draw()
    {	
    	for (int y = 0; y < BOARD_SIZE; y++)
    	{
    		for (int x = 0; x < BOARD_SIZE; x++)
    		{
    			if (isOpen(x, y))
    				drawOpen(x, y, getType(x, y), isDead(x, y));
    			else
    				drawClosed(x, y);
    		}
    	}
    
    	if (gameOver() || hasWon()) {
    		if (game_clock.getElapsedTime() > 0.25) {
    			static int toggle = 1;
    			toggle ^= 1;
    			openMines(toggle == 0);
    			game_clock.restart();
    		}
    	}
    }
    
    bool requestRestart(int x, int y)
    {
    	return (x >= 3 && x <= 5 && y >= 10 && y <= 12);
    }
    
    void init()
    {
    	for (int i = 0; i < BOARD_SIZE*BOARD_SIZE; i++) {
    		board[i].type = 0;
    		board[i].flag = false;
    		board[i].open = false;
    	}
    
    	for (int i = 0; i<MINE_COUNT; i++)
    	{
    		bool tmp = true;
    		do
    		{
    			int x = rand_int(0, BOARD_SIZE - 1);
    			int y = rand_int(0, BOARD_SIZE - 1);
    			if (!isMine(x, y))
    			{
    				tmp = false;
    				setType(x, y, MINE);
    			}
    		} while (tmp);
    	}
    
    	for (int y = 0; y < BOARD_SIZE; y++) {
    		for (int x = 0; x < BOARD_SIZE; x++) {
    			if (!isMine(x, y)) {
    				setType(x, y, calcMine(x, y));
    			}
    		}
    	}
    
    	death = -1;
    	clicked = true;
    	game_clock.restart();
    
    	num_opened = BOARD_SIZE*BOARD_SIZE;
    	glClearColor(0.8f, 0.8f, 0.8f, 1.f);
    	glMatrixMode(GL_PROJECTION);
    	glLoadIdentity();
    	glOrtho(0, width, 0, height, -1.f, 1.f);
    	glPointSize(5.0);
    	glEnable(GL_LINE_SMOOTH);
    	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    	glEnable(GL_POINT_SMOOTH);
    	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    	glEnable(GL_BLEND);
    	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
    // glut callbacks
    void display()
    {
    	glClear(GL_COLOR_BUFFER_BIT);
    	drawLowerFrame();
    	drawUpperFrame();
    	drawIcon();
    	draw();
    
    	glutSwapBuffers();
    }
    
    void key(unsigned char key, int x, int y)
    {
    	switch (key) {
    	case 27: exit(0); break;
    	}
    	//glutPostRedisplay();
    }
    
    void mouse(int b, int s, int x, int y)
    {
    	x = (x + PADDING) / TILE_SIZE - 1;
    	y = (height - y + PADDING) / TILE_SIZE - 1;
    
    	switch (b)
    	{
    	case GLUT_LEFT_BUTTON:
    		if (s == GLUT_DOWN)
    		{
    			if (requestRestart(x, y))
    			{
    				init();
    			}
    			else if (!gameOver() && !hasWon()) {
    				openCell(x, y);
    			}
    		}
    		break;
    	case GLUT_RIGHT_BUTTON:
    		if (s == GLUT_DOWN)
    		{
    			if (gameOver() || hasWon()) break;
    			toggleFlag(x, y);
    		}
    		break;
    	}
    
    	//glutPostRedisplay();
    }
    
    int main(int argc, char **argv)
    {
    	width = BOARD_SIZE*TILE_SIZE + 2 * PADDING;
    	height = BOARD_SIZE*TILE_SIZE + 2 * PADDING + 2 * MARGIN;
    
    	glutInit(&argc, argv);
    	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
    	glutInitWindowSize(width, height);
    	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - width) / 2, (glutGet(GLUT_SCREEN_HEIGHT) - height) / 2);
    	glutCreateWindow("minesweeper");
    	glutIdleFunc(display);
    	glutDisplayFunc(display);
    	glutKeyboardFunc(key);
    	glutMouseFunc(mouse);
    
    	init();
    
    	glutMainLoop();
    	return 0;
    }


  [1]: https://i.stack.imgur.com/6y5rU.png