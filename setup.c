/*  -------------------------------
	Working command to build this:
	-------------------------------

	gcc `pkg-config --cflags gtk+-3.0` -o setup setup.c `pkg-config --libs gtk+-3.0` -lglut -lGL
	./setup

	This one!!!!

	gcc `pkg-config --cflags gtk+-3.0` -o setup setup.c glad.c `pkg-config --libs gtk+-3.0` -lglut -lGL -lglfw3 -ldl -lm -lGL -lGLU -lX11 -pthread
	./setup

	This is a dummy application representing the layout of both the plasmas application and the heart application (the basics).
	Demo is basically just a menu at the top, a GLArea in the middle, and a menu at the bottom. Both menus just have some labels
	to illustrate where the real projects will actually display real values and/or buttons that actually have useful information
	or actually perform some real action.
*/

#include <gtk/gtk.h>
#include <GL/glut.h>


#include <GLFW/glfw3.h>

#include <stdio.h>
#include <math.h>

void init_buffer_objects(GError *error);
void init_shaders(GError *error);

static void on_realize (GtkGLArea *area);
static gboolean render(GtkGLArea *area, GdkGLContext *context);

unsigned int shaderProgram;

const char *vertexShaderSource ="#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n\n"
    "out vec3 ourColor;\n\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 ourColor;\n\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(ourColor, 1.0);\n"
    "}\0";

static void print_hello(GtkWidget *widget, gpointer data) {
	g_print("Hello World\n"); // print to a terminal if the application was started from one.
}

/*
	So basically this is going to be the method where we are putting the rendering stuff, where we redraw things.
*/
static gboolean render(GtkGLArea *area, GdkGLContext *context)
{
	/* Set clear color to black, clear screen.*/
    glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// draw stuff here - this MUST be modern OpenGL. Old OpenGL will NOT run, because GTK says it's...well...old.
	glUseProgram(shaderProgram);

    // update shader uniform
    double  timeValue = glfwGetTime();
    float greenValue = (float)(sin(timeValue) / 2.0 + 0.5);
    int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
    glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

    // render the triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);

	return TRUE;
}

/*
	This function sets up the window layout and adds all the controls.
*/
static void activate(GtkApplication *app, gpointer user_data)
{
	printf("hello activation....\n");
	// GtkWidget is the base class that all widgets in GTK+ derive from. Manages widget lifestyle, states, and style.
	GtkWidget *window = gtk_application_window_new(app);
	GtkWidget *grid = gtk_grid_new();
	GtkWidget *label = gtk_label_new("Hello Label World!");
	GtkWidget *button = gtk_button_new_with_label("Hello World of Buttons...");
	GtkWidget *gl_area = gtk_gl_area_new();
	GtkWidget *label2 = gtk_label_new("Label 1 at the bottom...Time");
	GtkWidget *label3 = gtk_label_new("Label 2 at the bottom...# Dust grains");

	gtk_grid_set_column_spacing(GTK_GRID(grid), 50);
	gtk_grid_set_row_spacing(GTK_GRID(grid), 10);

	/* -------- Customizing a few things: setting title, changing size of window, entering the window on the screen --------*/

	// doing this: GTK_WINDOW(window) casts the window (which is a pointer to a GtkWidget object) to a GtkWindow - GTK_WINDOW is a macro.

	gtk_window_set_title(GTK_WINDOW(window), "GTK Tutorial");	// specify window with GTK_WINDOW(window), and pass title to display
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 800);	// specify window with GTK_WINDOW(window), and pass width, height
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), button, 1, 0, 1, 1);
	int rowSpanOfGLArea = 50;  // Going to have to play around with this...GTK is a little unfriendly in terms of sizing.
	gtk_grid_attach(GTK_GRID(grid), gl_area, 0, 1, 2, rowSpanOfGLArea);
	gtk_grid_attach(GTK_GRID(grid), label2, 0, rowSpanOfGLArea+1, 1, 1);

	// Some demo button functionality.
	g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);  /* print_hello is the event handler, NULL because print_hello
										doesn't take any data.*/
	g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_widget_destroy), window);
	// the swapped version of g_signal,_connect allows the callback function to take a parameter passed in as data.

	// Connect to the render signal and connect to the realize signal, which are the render functions and initialization functions, respectively.
	g_signal_connect(gl_area, "render", G_CALLBACK (render), NULL);
	g_signal_connect(gl_area, "realize", G_CALLBACK(on_realize), NULL);

	gtk_container_add(GTK_CONTAINER(window), grid); /* this will actually add the button to the window (technically the button_box,
	//							 but the button_box contains the button */
	gtk_widget_show_all(window);
}

// Used to initialize OpenGL state, e.g. buffer objects or shaders.
static void on_realize (GtkGLArea *area)
{
	// We need to make the context current if we want to call GL API
	gtk_gl_area_make_current (area);

	// If there were errors during the initialization or when trying to make the context current, this
	// function will return a GError for you to catch
	// If error is UnNULL, meaning it isn't not set (double negative), it is set, return and see what error is.
	if (gtk_gl_area_get_error (area) != NULL) {
		return;
	}

	// You can also use gtk_gl_area_set_error() in order to show eventual initialization errors on the
	// GtkGLArea widget itself
	GError *error = NULL;
	init_buffer_objects (&error);
	if (error != NULL)
	{
	    gtk_gl_area_set_error (area, error);
	    g_error_free (error);
	    return;
	}

	init_shaders (&error);
	if (error != NULL)
    {
      gtk_gl_area_set_error (area, error);
      g_error_free (error);
      return;
    }
	printf("OnRealize function ended\n");
}

/*
	This function has one purpose which is to buffer the triangle data--it does all the VAO and VBO stuff
	which to be fair, I'm not completely sure how it works in detail.
*/
void init_buffer_objects(GError *errorObj) {
	// set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions
        // colors
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom left
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f // top
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Tell OpenGL how the data is formatted for the vertex attribute
    // 0 - index of attribute, we are modifying vertex attribute which we set to index 0
    // 3 - size, so when we read, how many elements to read at a time.
    // GL_FLOAT - just specifies we are reading floats.
    // GL_FALSE - read them as fixed point values (whatever that means)
    // 6 * sizeof(float) = 24 - byte offset, that's how many bytes as a group to read at a time.
    // (void*)0 - pointer for element to start at, we are starting at 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Tell OpenGL how the data is formatted for the color attribute
    // Last attribute is different, because we are starting at 4th float, or index 3, which is byte 12.
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    //glBindVertexArray(0);

    // bind the VAO (it was already bound, but just to demonstrate): seeing as we only have a single VAO we can
    // just bind it beforehand before rendering the respective triangle; this is another approach.
    glBindVertexArray(VAO);

}

/*
	This function has one purpose, to compile the shaders for the application. In this demo, it just glCompileShader
	the shaders for the colorful triangle.
*/
void init_shaders(GError *errorObj) {
	// build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
    }
    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n");
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

/*
	Keep this clean - this is just where we create an application and start it up. We do connect the activate signal,
	but that's about as fancy as we need to get here. It's not that you can't, it's just much cleaner if you do the
	other stuff in its own dedicated spot.
*/
int main(int argc, char *argv[])
{
	GtkApplication *app;
	int status;
	app = gtk_application_new("edu.tarleton.pmg.complex-plasmas", G_APPLICATION_FLAGS_NONE);	// create a new application (just a container to hold everything)
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL); // This will cause the activate function we created to be called
	// App created, activate signal connected, it can be ran now.
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app); // Tidy up and free the memory when we are through.
	return 0;
}
