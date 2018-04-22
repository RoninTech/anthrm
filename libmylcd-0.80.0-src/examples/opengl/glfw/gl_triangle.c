//========================================================================
// This is a small test application for GLFW.
// The program opens a window (640x480), and renders a spinning colored
// triangle (it is controlled with both the GLFW timer and the mouse). It
// also calculates the rendering speed (FPS), which is displayed in the
// window title bar.
//========================================================================

#include <stdio.h>
#include <windows.h>
#include "../mylcdgl.h"

static int width, height;


void GLFWCALL Resize ( int x, int y )
{
    width = x;
    height = y > 0 ? y : 1;   // Prevent division by zero in aspect calc.
	mylcdgl_SetSize(width, height);
	glViewport( 0, 0, width, height );
	
	glMatrixMode( GL_MODELVIEW );

	glLoadIdentity();
	gluPerspective( 65.0f, (GLfloat)x/(GLfloat)y, 1.0f,        100.0f );

        // Select and setup the modelview matrix
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	gluLookAt(0.0f, 1.0f, 0.0f,    // Eye-position
			  0.0f, 20.0f, 0.0f,   // View-point
			  0.0f, 0.0f, 1.0f );  // Up-vector
	
}

//========================================================================
// main()
//========================================================================

int main( void )
{
    int     running, frames, x=0;
    double  t, t0, fps;


    // Initialise GLFW
    glfwInit();
	mylcdgl_update(MYGL_CREATECONTEXT, -1, -1);
    // Open OpenGL window
    /*if( !glfwOpenWindow( DWIDTH, DHEIGHT, 0,0,0,0, 0,0, GLFW_WINDOW ) ){
        glfwTerminate();
        return 0;
    }*/

    // Enable sticky keys
    //glfwEnable( GLFW_STICKY_KEYS );
    //glfwSetWindowSizeCallback( Resize );
    Resize(DWIDTH, DHEIGHT);
	width = DWIDTH;
	height = DHEIGHT;
    // Disable vertical sync (on cards that support it)
    glfwSwapInterval( 0 );

    // Main loop
    running = GL_TRUE;
    frames = 0;
    t0 = glfwGetTime();
    
    while(running && !kbhit()){
        // Get time and mouse position
        t = glfwGetTime();
        //glfwGetMousePos( &x, &y );

        // Calculate and display FPS (frames per second)
        if( (t-t0) > 1.0 ){
            fps = (double)frames / (t-t0);
            printf("Spinning Triangle (%.1f FPS)\n", fps);
           // glfwSetWindowTitle( titlestr );
            t0 = t;
            frames = 0;
        }
        frames++;

        // Clear color buffer
        glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
        glClear( GL_COLOR_BUFFER_BIT );


        // Select and setup the projection matrix
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        gluPerspective( 65.0f, (GLfloat)width/(GLfloat)height, 1.0f,   100.0f );


        // Select and setup the modelview matrix
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
        gluLookAt( 0.0f, 1.0f, 0.0f,    // Eye-position
                   0.0f, 20.0f, 0.0f,   // View-point
                   0.0f, 0.0f, 1.0f );  // Up-vector

        // Draw a rotating colorful triangle
        glPushMatrix();
        glTranslatef( 0.0f, 14.0f, 0.0f );
        glRotatef( 0.3*(GLfloat)x + (GLfloat)t*100.0f, 0.0f, 0.0f, 1.0f );
        glBegin( GL_TRIANGLES );
          glColor3f( 1.0f, 0.0f, 0.0f );
          glVertex3f( -5.0f, 0.0f, -4.0f );
          glColor3f( 0.0f, 1.0f, 0.0f );
          glVertex3f( 5.0f, 0.0f, -4.0f );
          glColor3f( 0.0f, 0.0f, 1.0f );
          glVertex3f( 0.0f, 0.0f, 6.0f );
        glEnd();
        glPopMatrix();

        // Swap buffers
        //glfwSwapBuffers();
		mylcdgl_update(0, -1, -1);
		lSleep(30);
		
        // Check if the ESC key was pressed or the window was closed
        //running = !glfwGetKey( GLFW_KEY_ESC ) && glfwGetWindowParam( GLFW_OPENED );
    }
    
	mylcdgl_shutdown();
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
