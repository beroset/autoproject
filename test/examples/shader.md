# [Modern Opengl GLSL Shader Class](https://codereview.stackexchange.com/questions/92924)
### tags: ['c++', 'c++11', 'opengl']

I've finally written my own `Shader` class. It's a basic implementation based on a tutorial, and it works perfectly. Any suggestions on how to improve, or correct my code are welcome!

**Shader.h**

    #pragma once
    
    #include <GL/glew.h>
    #include <map>
    #include <string>
    #include "LogManager.h"
    #include "bindable.h"
    #include "disposable.h"
    
    #define NUM_SHADER_TYPES 4
    
    class Shader : public Bindable, public Disposable
    {
    public:
        Shader();
        virtual ~Shader();
    
        void loadFromText(GLenum type, const std::string& src);
        void loadFromFile(GLenum type, const char* fileName);
        void loadFromPreCompiledText(GLenum type, const std::string& src){}
        void loadFromPreCompiledFile(GLenum type, const char* fileName){}
        void CreateAndLink();
        void RegisterAttribute(const char* attrib);
        void RegisterUniform(const char* uniform);
        GLuint GetProgramID() const;
        ///accesses elements : shaders/uniforms;
        GLuint GetAttribLocation(const char* attrib);
        GLuint operator[](const char* attrib);
        GLuint GetUniformLocation(const char* unif);
        GLuint operator()(const char* unif);
    
        virtual void Bind() const;
        virtual void UnBind() const;
        virtual void Dispose();
    
    private:
        enum ShaderType { VERTEX_SHADER, FRAGMENT_SHADER, GEOMETRY_SHADER, PIXEL_SHADER};
        GLuint _program ;
        int _numShaders;
        GLuint _shaders[4]; /// VERTEX, FRAGMENT, GEOMETRY AND PIXEL_SHADERS !
        std::map<std::string, GLuint> _attribList;
        std::map<std::string, GLuint> _unifLocationList;
    };

**Shader.cpp**

    #include "Shader.h"
    #include "LogManager.h"
    #include "fstream"
    
    Shader::Shader()
        :_program(0), _numShaders(0)
    {
        _shaders[VERTEX_SHADER] = 0;
        _shaders[FRAGMENT_SHADER] = 0;
        _shaders[GEOMETRY_SHADER] = 0;
        _shaders[PIXEL_SHADER] = 0;
        _attribList.clear();
        _unifLocationList.clear();
    }
    
    Shader::~Shader(){
        _attribList.clear();
        _unifLocationList.clear();
    }
    
    void Shader::loadFromText(GLenum type, const std::string& text){
        GLuint shader = glCreateShader(type);
        const char* cstr = text.c_str();
        glShaderSource(shader, 1, &cstr, nullptr);
    
        ///compile + check shader load status
        GLint status;
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE){
            GLint infoLogSize;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogSize);
            GLchar *infoLog = new GLchar[infoLogSize];
            glGetShaderInfoLog(shader, infoLogSize, nullptr, infoLog);
            LOG_ERROR("Shader", infoLog);
            delete [] infoLog;
        }
        _shaders[_numShaders++]=shader;
    }
    
    void Shader::CreateAndLink(){
        _program = glCreateProgram();
        if(_shaders[VERTEX_SHADER] != 0)
            glAttachShader(_program, _shaders[VERTEX_SHADER]);
        if(_shaders[FRAGMENT_SHADER] != 0)
            glAttachShader(_program, _shaders[FRAGMENT_SHADER]);
        if(_shaders[GEOMETRY_SHADER] != 0)
            glAttachShader(_program, _shaders[GEOMETRY_SHADER]);
        if(_shaders[PIXEL_SHADER] != 0)
            glAttachShader(_program, _shaders[PIXEL_SHADER]);
    
        ///link + check
        GLint status;
        glLinkProgram(_program);
        glGetProgramiv(_program, GL_LINK_STATUS, &status);
        if(status == GL_FALSE){
            GLint infoLogSize;
            glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infoLogSize);
            GLchar *infoLog = new GLchar[infoLogSize];
            glGetProgramInfoLog(_program, infoLogSize, nullptr, infoLog);
            delete [] infoLog;
        }
        
        glDetachShader(_program, _shaders[VERTEX_SHADER]);
        glDetachShader(_program, _shaders[FRAGMENT_SHADER]);
        glDetachShader(_program, _shaders[GEOMETRY_SHADER]);
        glDetachShader(_program, _shaders[PIXEL_SHADER]);
        
        glDeleteShader(_shaders[VERTEX_SHADER]);
        glDeleteShader(_shaders[FRAGMENT_SHADER]);
        glDeleteShader(_shaders[GEOMETRY_SHADER]);
        glDeleteShader(_shaders[PIXEL_SHADER]);
    }
    
    void Shader::Bind() const{
        glUseProgram(_program);
    }
    
    void Shader::UnBind() const{
        glUseProgram(0);
    }
    
    void Shader::RegisterAttribute(const char* attrib){
        _attribList[attrib] = glGetAttribLocation(_program, attrib);
    }
    
    void Shader::RegisterUniform(const char* unif){
        _unifLocationList[unif] = glGetUniformLocation(_program, unif);
    }
    
    GLuint Shader::GetAttribLocation(const char* attrib){
        return _attribList[attrib];
    }
    GLuint Shader::operator[](const char* attrib){
        return _attribList[attrib];
    }
    
    GLuint Shader::GetUniformLocation(const char* unif){
        return _unifLocationList[unif];
    }
    GLuint Shader::operator()(const char* unif){
        return _unifLocationList[unif];
    }
    
    GLuint Shader::GetProgramID() const{ return _program; }
    
    void Shader::loadFromFile(GLenum which, const char* fileName){
        std::ifstream fparser;
        fparser.open(fileName, std::ios_base::in);
        if(fparser){
            ///read + load
            std::string buffer(std::istreambuf_iterator<char>(fparser), (std::istreambuf_iterator<char>()));
            loadFromText(which, buffer);
        }
        else{
            LOG_ERROR_INFO("Shader", "Invalid fileName path", fileName);
        }
    }
    
    void Shader::Dispose(){
        glDeleteProgram(_program);
        _program = -1;
    }