the problem is a code review and not just an openGL related.
I can't explain the problem in brief so here is the class code and the main() function and i'll comment the problem.

# shaderManager.h

    #include <iostream>
    #include <sstream>
    #include <string>
    #include <fstream>
    
    #include "glew.h"
    #include "glfw3.h"
    
    class shader{
    	private:
    	GLchar	infolog[512];
    	const GLchar* FragShaderPath;
    	const GLchar* VertShaderPath;
    	const GLchar* FragShaderCode;
    	const GLchar* VertShaderCode;
    	GLuint FragShader;
    	GLuint VertShader;
    	GLuint ShaderProgram;
    	
    	public:
    	shader(const GLchar* VSPath,const GLchar* FSPath);
    	const GLchar* getFragShaderCode();
    	const GLchar* getVertShaderCode();
    	void load();
    	void compile();
    	void use();
    	void clear();
    	
    };

# shaderManager.cpp

    #include "shaderManager.h"
    
    using namespace std;
    
    void shader::load(){
    	ifstream VertShaderFile;
    	ifstream FragShaderFile;
    	stringstream FragShaderSS;
    	stringstream VertShaderSS;
    	string VertShaderContent;
    	string FragShaderContent;
    	VertShaderFile.exceptions(ifstream::failbit|ifstream::badbit);
    	FragShaderFile.exceptions(ifstream::failbit|ifstream::badbit);
    	try{
    		FragShaderFile.open(this->FragShaderPath);
    		FragShaderSS << FragShaderFile.rdbuf();
    		FragShaderContent = FragShaderSS.str();
    		FragShaderFile.close();
    		
    		VertShaderFile.open(this->VertShaderPath);
    		VertShaderSS << VertShaderFile.rdbuf();
    		VertShaderContent = VertShaderSS.str();
    		VertShaderFile.close();
    	}catch(ifstream::failure e){
    		cout<<"error while loading vertex or fragment file";
    	}
    	this->VertShaderCode = VertShaderContent.c_str();
    	this->FragShaderCode = FragShaderContent.c_str();
    	
    	//now i will use std::cout<< to be sure that data has been loaded when i call this methode in the main() function
    	cout<<this->VertShaderCode<<endl;
    	cout<<this->FragShaderCode<<endl;
    }
    
    void shader::compile(){
    		//now the problem i think will occure, this->VertShaderCode and this->FragShaderCode apear to contain no data !! were they gone
    		//what's wrong?? let see.
    		
    		cout<<this->VertShaderCode;
    		cout<<this->FragShaderCode;
    		this->VertShader = glCreateShader(GL_VERTEX_SHADER);
    		glShaderSource(this->VertShader,1,&this->VertShaderCode,NULL);
    		glCompileShader(this->VertShader);
    		GLint success;
    		
    		glGetShaderiv(this->VertShader,GL_COMPILE_STATUS,&success);
    		if(!success){
    			glGetShaderInfoLog(this->VertShader,512,NULL,this->infolog);
    			cout<<"Vertex Shader Compilation Errore "<<this->infolog<<endl;
    		}
    		
    		this->FragShader = glCreateShader(GL_FRAGMENT_SHADER);
    		glShaderSource(this->FragShader,1,&this->FragShaderCode,NULL);
    		glCompileShader(this->FragShader);
    		glCompileShader(this->FragShader);
    		
    		glGetShaderiv(this->FragShader,GL_COMPILE_STATUS,&success);
    		if(!success){
    			glGetShaderInfoLog(this->FragShader,512,NULL,this->infolog);
    			cout<<"Fragment Shader Compilation Errore "<<this->infolog<<endl;
    		}
    		
    		this->ShaderProgram = glCreateProgram();
    		glAttachShader(this->ShaderProgram,this->FragShader);
    		glAttachShader(this->ShaderProgram,this->VertShader);
    		glLinkProgram(this->ShaderProgram);
    		
    		glGetProgramiv(this->ShaderProgram,GL_LINK_STATUS,&success);
    		if(!success){
    			glGetProgramInfoLog(this->ShaderProgram,512,NULL,this->infolog);
    			cout<<"program Linking Errore "<<this->infolog<<endl;
    		}
    }
    
    void shader::use(){
    	glUseProgram(this->ShaderProgram);
    }
    
    void shader::clear(){
    	glDeleteShader(this->VertShader);
    	glDeleteShader(this->FragShader);
    }
    
    shader::shader(const GLchar* VSPath,const GLchar* FSPath){
    	this->FragShaderPath = FSPath;
    	this->VertShaderPath = VSPath;
    }
    
    const GLchar* shader::getVertShaderCode(){
    	return this->VertShaderCode;
    }
    
    const GLchar* shader::getFragShaderCode(){
    	return this->FragShaderCode;
    }

# main.cpp

    #include "inc/primitivesRenderers.h"
    #include "inc/shaderManager.h"
    
    void key_callback(GLFWwindow* window,int key,int scancode,int action,int mode);
    
    GLfloat vertices[] = {
    // Positions 	// Colors	 // Texture Coords
    0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // Top Right
    0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Bottom Right
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // Bottom Left
    -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f // Top Left
    };
    
    using namespace std;
    
    int main(){
    	if(!glfwInit()){
    		std::cout<<"error: failed to init glfw"<<std::endl;
    		return -1;
    	}
    	
    	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    	
    	GLFWwindow* window = glfwCreateWindow(800,600,"streater",nullptr,nullptr);
    	glfwSetKeyCallback(window,key_callback);
    	
    	if(window == nullptr){
    		std::cout<<"Failed to create GLFW window"<<std::endl;
    		glfwTerminate();
    		return -1;
    	}
    	
    	glfwMakeContextCurrent(window);
    	
    	glewExperimental = GL_TRUE;
    	
    	if(glewInit() != GLEW_OK){
    		std::cout<<"failed to initilize glew"<<std::endl;
    		return -1;
    	}
    	
    	glViewport(0,0,800,600);
    	
    	shader myshaderobject = shader("vshader.txt","fshader.txt");
    	myshaderobject.load();
        // after load method i see the myshaderobject.VertShaderCode and myshaderobject.FragShaderCode as i planned to in shader::load()
    	
    	myshaderobject.compile();
        //after compile method i cant see the myshaderobject.VertShaderCode and myshaderobject.FragShaderCode as i planned to in shder::compile() and as expected some openGL shader compilation errores
    	
    	//also here i think contain no data
    	cout<<myshaderobject.getFragShaderCode()<<endl;
    	cout<<myshaderobject.getVertShaderCode()<<endl;
    	
    	model mdl = model(vertices);
    	
    	while(!glfwWindowShouldClose(window)){
    		glfwPollEvents();
    		//rendering command here
            //myshaderobject.use(); but its not compiled
    		mdl.draw();
    		glClear(GL_COLOR_BUFFER_BIT);
    		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    
    		glfwSwapBuffers(window);
    	}
    	
    	glfwTerminate();
    	
    	return 0;
    }
    
    void key_callback(GLFWwindow* window,int key,int scancode,int action,int mode){
    	if(key == GLFW_KEY_ESCAPE && action==GLFW_PRESS)
    		glfwSetWindowShouldClose(window,GL_TRUE);
    }

-------------compilation commandline---------------

    C:\Users\nidal\Desktop\streater>g++ main.cpp -std=c++11 "inc/shaderManager.cpp" "inc/primitivesRenderers.cpp" -L"lib" -lglfw3dll -lglew32.dll -lopengl32

--------------------program output-----------------

    C:\Users\nidal\Desktop\streater>a
    #version 330 core
    layout (location = 0) in vec3 position;
    void main()
    {
    gl_Position = vec4(position.x, position.y, position.z, 1.0);
    }//this is the content of myshaderobject.VertShaderCode
    
    #version 330 core
    out vec4 color;
    void main()
    {
    color = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    };//this is the content of myshaderobject.FragShaderCode
    
    //this two last output are the result to the call of myshaderobject.load
    
    └ 
    └
    // this two last output are the call to myshaderobject.compile
    
    Vertex Shader Compilation Errore ERROR: 0:1: '' :  illegal non-ASCII character
    (0xc0)
    
    
    Fragment Shader Compilation Errore ERROR: 0:1: '' :  illegal non-ASCII character
     (0xc0)
    
    
    program Linking Errore Attached vertex shader is not compiled.
    
    └ //the call to cout<<myshaderobject.getFragShaderCode()<<endl;
    └ //the call to cout<<myshaderobject.getVertShaderCode()<<endl;
