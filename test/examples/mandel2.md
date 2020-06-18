As an exercise to learn how OpenGL and image creation worked, as well as to satisfy a curiosity I've developed for Chaos Theory, I decided to create a mandelbrot fractal drawer in C++, which can either draw to an OpenGL context or to a PNG image. I am a beginner at C++, and would much appreciate any constructive feedback. Also, please ridicule me on how I create my images, and how I can make them look better as well.

# main.cpp

    #include <complex>
    #include <iostream>
    #include <memory>
    #include "Window.h"
    #include "Draw_Buffer.h"
    #include "Image_Buffer.h"
    #include "Buffer_Base.h"
    
    static constexpr float COMPLEX_INCREMENT = 0.005f;
    
    template <typename T>
    int iterations_till_escape(const std::complex<T> &c, int max_iterations) {
        std::complex<T> z(0, 0);
        for (int iter = 0; iter < max_iterations; ++iter) {
            z = (z * z) + c;
            if (std::abs(z) > 2) {
                return iter;
            }
        }
        return -1;
    }
    
    template <typename T>
    RGB calculate_pixel(const std::complex<T> &c) {
        int iterations = iterations_till_escape(c, 255);
    
        if (iterations == -1) {
            return RGB{0, 0, 0};
        }
    
        else {
            GLubyte blue = iterations * 5;
            return RGB{0, 0, blue};
        }
    }
    
    int main() {
        // Declare window object to represent the complex plane
        Window<float> complex_plane(-2.2, 1.2, -1.7, 1.7);
    
        // Declare window object to represent the OpenGL window
        Window<int> window(0, ((std::abs(complex_plane.get_x_min()) + complex_plane.get_x_max()) / COMPLEX_INCREMENT),
                           0, ((std::abs(complex_plane.get_y_min()) + complex_plane.get_y_max()) / COMPLEX_INCREMENT));
    
        std::unique_ptr<Buffer_Base<RGB>> pixel_buffer;
    
    
        std::cout << "Running mandelbrot-fractal-drawer...\nWould you like to draw fractal to a window or an image?\n"
                  << "Type W for window or I for image" << std::endl;
    
        char response;
        while (!(std::cin >> response))
            ;
        if (response == 'W' || response == 'w') {
            // Initialise pointer to a draw buffer
            pixel_buffer.reset(new Draw_Buffer(&window, "vertex_shader.glsl", "fragment_shader.glsl"));
        }
    
        else if (response == 'I' || response == 'i') {
            std::cout << "\nPlease enter the location to where you want the fractal to be drawn" << std::endl;
    
            std::string src;
            while (!(std::cin >> src))
                ;
    
            // Initialise pointer to an image buffer
            pixel_buffer.reset(new Image_Buffer(&window, src));
        }
    
        std::complex<float> pixel_iterator(complex_plane.get_x_min(), complex_plane.get_y_max());
        while (pixel_iterator.imag() > complex_plane.get_y_min()) {
            while (pixel_iterator.real() < complex_plane.get_x_max()) {
    
                // Calculate the colour of the pixel using the mandelbrot function
                *pixel_buffer << calculate_pixel(pixel_iterator);
    
                // Increment
                pixel_iterator.real(pixel_iterator.real() + COMPLEX_INCREMENT);
            }
    
            // Increment
            pixel_iterator.imag(pixel_iterator.imag() - (COMPLEX_INCREMENT));
    
            // Reset real iterator
            pixel_iterator.real(complex_plane.get_x_min());
        }
    
        pixel_buffer->flush();
    
    
        std::cout << "Closing down..." << std::endl;
    
    }

# Buffer_Base.h

    #ifndef MANDELBROT_FRACTAL_DRAWER_BUFFER_BASE_H
    #define MANDELBROT_FRACTAL_DRAWER_BUFFER_BASE_H
    
    #include <vector>
    #include <memory>
    
    #include "Window.h"
    
    template <typename T>
    class Buffer_Base {
    protected:
        // The buffer itself
        std::vector<T> buffer;
    
        // Iterator to where in the buffer the appending is happening
        typename std::vector<T>::iterator pos_iter;
    
        // Represents the size of the window to which the buffer is writing
        std::unique_ptr<Window<int>> window;
    public:
        Buffer_Base(Window<int> *win) :
                buffer(win->size()), window(win) { pos_iter = buffer.begin(); }
        virtual ~Buffer_Base() { };
        virtual void flush() = 0;
    
        Buffer_Base<T> &operator<<(T &&val) {
            if (pos_iter != buffer.end()) {
                *(pos_iter) = std::move(val);
                ++pos_iter;
            }
            return *this;
        }
    };
    
    #endif //MANDELBROT_FRACTAL_DRAWER_BUFFER_BASE_H

# RGB.h

    #ifndef MANDELBROT_FRACTAL_DRAWER_RGB_H
    #define MANDELBROT_FRACTAL_DRAWER_RGB_H


    struct RGB {
        unsigned char r;
        unsigned char g;
        unsigned char b;
    };
    
    #endif //MANDELBROT_FRACTAL_DRAWER_RGB_H

# Get_GL.h


    #ifndef MANDELBROT_FRACTAL_DRAWER_GET_GL_H
    #define MANDELBROT_FRACTAL_DRAWER_GET_GL_H
    
    #ifndef __APPLE__
    #include <GL/gl.h>
    #else
    #include <OpenGL/gl.h>
    #endif
    
    #endif //MANDELBROT_FRACTAL_DRAWER_GET_GL_H

# Window.h

    #ifndef MANDELBROT_FRACTAL_DRAWER_WINDOW_H
    #define MANDELBROT_FRACTAL_DRAWER_WINDOW_H
    
    #include <complex>
    
    template<typename T>
    class Window {
        T _x_min, _x_max, _y_min, _y_max;
    public:
        Window(T x_min, T x_max, T y_min, T y_max) : _x_min(x_min), _x_max(x_max), _y_min(y_min), _y_max(y_max) { }
    
    // Util functions
        T width() const {
            return (_x_max - _x_min);
        }
    
        T height() const {
            return (_y_max - _y_min);
        }
    
        T size() const {
            return (height() * width());
        }
    
    // Setters and getters
        T get_y_min() const {
            return _y_min;
        }
    
        T get_y_max() const {
            return _y_max;
        }
    
        T get_x_min() const {
            return _x_min;
        }
    
        T get_x_max() const {
            return _x_max;
        }
    
        void set_y_min(T _y_min) {
            Window::_y_min = _y_min;
        }
    
        void set_y_max(T _y_max) {
            Window::_y_max = _y_max;
        }
    
        void set_x_min(T _x_min) {
            Window::_x_min = _x_min;
        }
    
        void set_x_max(T _x_max) {
            Window::_x_max = _x_max;
        }
    
    // Reset values
        void reset(T x_min, T x_max, T y_min, T y_max) {
            _y_min(y_min);
            _y_max(y_max);
            _x_min(x_min);
            _x_max(x_max);
        }
    };
    #endif // MANDELBROT_FRACTAL_DRAWER_WINDOW_H

# Image_Buffer.h

    #ifndef MANDELBROT_FRACTAL_DRAWER_IMAGE_BUFFER_H
    #define MANDELBROT_FRACTAL_DRAWER_IMAGE_BUFFER_H
    
    #include <string>
    #include "Buffer_Base.h"
    #include "RGB.h"
    #include <png.h>
    
    #define PNG_DEBUG 3
    
    class Image_Buffer : public Buffer_Base<RGB> {
        // Location to write image to
        std::string file_src;
    
        // PNG data
        png_structp png_ptr;
        png_infop info_ptr;
        png_bytep row;
    
    
        // File pointer
        FILE *fp;
    public:
        Image_Buffer(Window<int> *, const std::string &);
    
        ~Image_Buffer();
    
        virtual void flush() override;
    };
    
    
    #endif //MANDELBROT_FRACTAL_DRAWER_IMAGE_BUFFER_H

# Image_Buffer.cpp

    #include "Image_Buffer.h"
    #include <png.h>
    #include <fstream>
    #include <stdexcept>
    #include <string>
    #include <sstream>
    #include <vector>
    #include <algorithm>
    
    Image_Buffer::Image_Buffer(Window<int> *win, const std::string &src) : Buffer_Base(win), file_src(src) { }
    
    void Image_Buffer::flush() {
        fp = fopen(file_src.c_str(), "wb");
        if (!fp) {
            std::ostringstream ss;
            ss << "error: Unable to open file " << file_src << " for writing";
            throw std::runtime_error(ss.str());
        }
    
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    
        if (!png_ptr) {
            throw std::runtime_error("error: png_create_write_struct failed");
        }
    
        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            throw std::runtime_error("error: png_create_info_struct failed");
        }
    
        if (setjmp(png_jmpbuf(png_ptr))) {
            throw std::runtime_error("Error during init_io");
        }
    
        png_init_io(png_ptr, fp);
    
        // Write header (8 bit colour depth)
        png_set_IHDR(png_ptr, info_ptr, window->width(), window->height(),
                     8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    
        png_text title_text;
        title_text.compression = PNG_TEXT_COMPRESSION_NONE;
        title_text.key = "Title";
        title_text.text = (char *)file_src.c_str();
        png_set_text(png_ptr, info_ptr, &title_text, 1);
    
        png_write_info(png_ptr, info_ptr);
    
        std::vector<RGB> row(3 * window->width());
        auto first = buffer.begin();
        auto last = buffer.begin() + window->width();
    
        while (first != buffer.end()) {
            std::copy(first, last, row.begin());
            png_write_row(png_ptr, (png_bytep)&row[0]);
            first = last;
            last += window->width();
        }
    
    
    
        png_write_end(png_ptr, NULL);
    
        png_init_io(png_ptr, fp);
    }
    
    Image_Buffer::~Image_Buffer() {
        if (fp) fclose(fp);
        if (info_ptr) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
        if (png_ptr) png_destroy_write_struct(&png_ptr, static_cast<png_infopp>(NULL));
    }

# Draw_Buffer.h

    #ifndef MANDELBROT_FRACTAL_DRAWER_DRAW_BUFFER_H
    #define MANDELBROT_FRACTAL_DRAWER_DRAW_BUFFER_H
    
    #define GLEW_STATIC
    
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
    #include "Get_GL.h"
    #include "Buffer_Base.h"
    #include "RGB.h"
    
    class Draw_Buffer : public Buffer_Base<RGB> {
        // Pointer to glfw screen
        GLFWwindow *screen;
    
        // Texture where pixels are written to
        GLuint mandelbrot_tex;
    
        // GLSL Shader program
        GLuint shader_prog;
    
        // Vertex shader
        GLuint vertex_shader;
    
        // Fragment shader
        GLuint frag_shader;
    
        // VAO
        GLuint vao;
    
        // Element buffer object
        GLuint ebo;
    
        // Vertex buffer object
        GLuint vbo;
    
        // Util function to compile shader
        static void compile_shader(GLuint &shader, const std::string &src);
    public:
        Draw_Buffer(Window<int> *, const std::string &, const std::string &);
        virtual ~Draw_Buffer() override;
    
        void make_current() {
            glfwMakeContextCurrent(screen);
        }
    
        virtual void flush() override;
    };
    
    
    #endif //MANDELBROT_FRACTAL_DRAWER_DRAW_BUFFER_H

# Draw_Buffer.cpp

    #include "Draw_Buffer.h"
    #include "Buffer_Base.h"
    #include <memory>
    #include <algorithm>
    #include <stdexcept>
    #include <vector>
    #include <sstream>
    #include <fstream>
    #include <string>
    #include <iostream>
    
    // Util function to compile a shader from source
    void Draw_Buffer::compile_shader(GLuint &shader, const std::string &src) {
        std::ifstream is(src);
        std::string code;
    
        std::string temp_str;
        while (std::getline(is, temp_str)) {
            code += temp_str + '\n';
        }
    
        const char *c_code = code.c_str();
        glShaderSource(shader, 1, &c_code, NULL);
        glCompileShader(shader);
    }
    
    Draw_Buffer::Draw_Buffer(Window<int> *win, const std::string &vertex_shader_src, const std::string &frag_shader_src) :
            Buffer_Base(win) {
    // Initialise GLFW
        if (!glfwInit()) {
            throw std::runtime_error("error: GLFW unable to initialise");
        }
    
    // Set up the window
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
        screen = (glfwCreateWindow(win->width(), win->height(), "Mandelbrot Fractal", nullptr, nullptr));
    
        make_current();
    
    // Initialise glew
        glewExperimental = GL_TRUE;
        GLenum glewinit = glewInit();
    
        if (glewinit != GLEW_OK) {
            std::ostringstream ss;
            ss << "error: Glew unable to initialise" << glewinit;
            throw std::runtime_error(ss.str());
        }
    
    // Clear
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
    
    // Generate shaders
        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    
        GLint compile_status;
        compile_shader(vertex_shader, vertex_shader_src);
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_status);
        if (compile_status != GL_TRUE) {
            char buffer[512];
            glGetShaderInfoLog(vertex_shader, 512, NULL, buffer);
            throw std::runtime_error(buffer);
        }
    
        compile_shader(frag_shader, frag_shader_src);
        glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &compile_status);
        if (compile_status != GL_TRUE) {
            char buffer[512];
            glGetShaderInfoLog(frag_shader, 512, NULL, buffer);
            throw std::runtime_error(buffer);
        }
    
    // Put shaders into shader program
        shader_prog = glCreateProgram();
        glAttachShader(shader_prog, vertex_shader);
        glAttachShader(shader_prog, frag_shader);
        glBindFragDataLocation(shader_prog, 0, "outColor");
        glLinkProgram(shader_prog);
        glUseProgram(shader_prog);
    
    // Create VAO
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
    
    // Create vertex and element buffers
        const static GLfloat vertices[] = {
                // Position   Tex-coords
                -1.0f,  1.0f, 0.0f, 0.0f, // Top-left
                 1.0f,  1.0f, 1.0f, 0.0f, // Top-right
                 1.0f, -1.0f, 1.0f, 1.0f, // Bottom-right
                -1.0f, -1.0f, 0.0f, 1.0f  // Bottom-left
        };
    
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
        const static GLuint elements[] = {
                0, 1, 2,
                2, 3, 0
        };
    
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
    
    // Set shader attributes
        GLint pos_attrib = glGetAttribLocation(shader_prog, "position");
        glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(pos_attrib);
    
        GLint tex_coord_attrib = glGetAttribLocation(shader_prog, "tex_coord");
        glEnableVertexAttribArray(tex_coord_attrib);
        glVertexAttribPointer(tex_coord_attrib, 2, GL_FLOAT, GL_FALSE,
                            4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    
    // Generate texture
        glGenTextures(1, &mandelbrot_tex);
    
    // Bind the texture information
    
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mandelbrot_tex);
        glUniform1i(glGetUniformLocation(shader_prog, "tex"), 0);
    }
    
    Draw_Buffer::~Draw_Buffer() {
    
    // Unbind buffer
        glBindVertexArray(0);
    
    // Delete shaders
        glDeleteProgram(shader_prog);
        glDeleteShader(vertex_shader);
        glDeleteShader(frag_shader);
    
    // Delete buffers
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);
    
    // Terminate GLFW
        glfwDestroyWindow(screen);
        glfwTerminate();
    }
    
    void Draw_Buffer::flush() {
        glClear(GL_COLOR_BUFFER_BIT);
    
        // Reset texture
        glBindTexture(GL_TEXTURE_2D, mandelbrot_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window->width(), window->height(), 0, GL_RGB, GL_BYTE, &buffer[0]);
    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
        // Draw rectangle
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
        // Sends message if there is an OpenGL bug
        GLenum err = glGetError();
        if (err) {
            std::stringstream ss;
            ss << "GL Error: " << err;
            throw std::runtime_error(ss.str());
        }
    
        // Swap buffers
        glfwSwapBuffers(screen);
    
        // Reset iterator
        pos_iter = buffer.begin();
    
        while(!glfwWindowShouldClose(screen)) {
            glfwPollEvents();
        }
    }

# fragment_shader.glsl

    #version 150
    
    in vec2 Tex_coord;
    
    out vec4 outColor;
    
    uniform sampler2D tex;
    
    void main() {
        outColor = texture(tex, Tex_coord);
    }

# vertex_shader.glsl

    #version 150
    
    in vec2 position;
    in vec2 tex_coord;
    
    out vec2 Tex_coord;
    
    void main() {
        gl_Position = vec4(position, 0.0, 1.0);
        Tex_coord = tex_coord;
    }
