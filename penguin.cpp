/***********************************************************
             CSC418/2504, Fall 2009
  
                 penguin.cpp
                 
       Simple demo program using OpenGL and the glut/glui 
       libraries

  
    Instructions:
        Please read the assignment page to determine 
        exactly what needs to be implemented.  Then read 
        over this file and become acquainted with its 
        design.

        Add source code where it appears appropriate. In
        particular, see lines marked 'TODO'.

        You should not need to change the overall structure
        of the program. However it should be clear what
        your changes do, and you should use sufficient comments
        to explain your code.  While the point of the assignment
        is to draw and animate the character, you will
        also be marked based on your design.

***********************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef _WIN32
#include <unistd.h>
#else
void usleep(unsigned int nanosec)
{
    Sleep(nanosec / 1000);
}
#endif


// *************** GLOBAL VARIABLES *************************

const float PI = 3.14159;
const float delta_theta = 0.01;

// --------------- USER INTERFACE VARIABLES -----------------

// Window settings
int windowID;               // Glut window ID (for display)
GLUI *glui;                 // Glui window (for controls)
int Win[2];                 // window (x,y) size


// ---------------- ANIMATION VARIABLES ---------------------

// Animation settings
int animate_mode = 0;       // 0 = no anim, 1 = animate
int animation_frame = 0;      // Specify current frame of animation

//////////////////////////////////////////////////////
// TODO: Add additional joint parameters here
//////////////////////////////////////////////////////

// Leg Joint parameters
const float LEG_JOINT_MIN = -30.0f;
const float LEG_JOINT_MAX =  30.0f;
float leg_rot = 0.0f;

// Feet Joint parameters
const float FOOT_JOINT_MIN =  -20.0f;
const float FOOT_JOINT_MAX =  5.0f;
float foot_rot = 0.0f;

// Arm Joint parameters
const float ARM_JOINT_MIN = -45.0f;
const float ARM_JOINT_MAX =  45.0f;
float arm_rot = 0.0f;

// Head Joint parameters
const float HEAD_JOINT_MIN = -5.0f;
const float HEAD_JOINT_MAX = 12.0f;
float head_rot = 0.0f;

// Beak Animated parameters
const float BEAK_JOINT_MIN = 0.0f;
const float BEAK_JOINT_MAX = 8.0f;
float beak_animate = 0.0f;

// Body Animated parameters
const float X_BODY_JOINT_MIN = -350.0f;
const float X_BODY_JOINT_MAX = 350.0f;
const float Y_BODY_JOINT_MIN = -20.0f;
const float Y_BODY_JOINT_MAX = 25.0f;
float x_body_animate = 350.0f;
float y_body_animate = 0.0f;


// ***********  FUNCTION HEADER DECLARATIONS ****************


// Initialization functions
void initGlut(char* winName);
void initGlui();
void initGl();


// Callbacks for handling events in glut
void myReshape(int w, int h);
void animate();
void display(void);

// Callback for handling events in glui
void GLUI_Control(int id);


// Functions to help draw the object
void drawPolygon(const float xVertices[], const float yVertices[], int vCount);
void drawCircle(float radius);
void drawFilledCircle(float radius);


// Return the current system clock (in seconds)
double getTime();


// ******************** FUNCTIONS ************************


// main() function
// Initializes the user interface (and any user variables)
// then hands over control to the event handler, which calls 
// display() whenever the GL window needs to be redrawn.
int main(int argc, char** argv)
{

    // Process program arguments
    if(argc != 3) {
        printf("Usage: demo [width] [height]\n");
        printf("Using 600x400 window by default...\n");
        Win[0] = 800;
        Win[1] = 600;
    } else {
        Win[0] = atoi(argv[1]);
        Win[1] = atoi(argv[2]);
    }


    // Initialize glut, glui, and opengl
    glutInit(&argc, argv);
    initGlut(argv[0]);
    initGlui();
    initGl();

    // Invoke the standard GLUT main event loop
    glutMainLoop();

    return 0;         // never reached
}


// Initialize glut and create a window with the specified caption 
void initGlut(char* winName)
{
    // Set video mode: double-buffered, color, depth-buffered
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Create window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Win[0],Win[1]);
    windowID = glutCreateWindow(winName);

    // Setup callback functions to handle events
    glutReshapeFunc(myReshape); // Call myReshape whenever window resized
    glutDisplayFunc(display);   // Call display whenever new frame needed 
}


// Quit button handler.  Called when the "quit" button is pressed.
void quitButton(int)
{
  exit(0);
}

// Animate button handler.  Called when the "animate" checkbox is pressed.
void animateButton(int)
{
  // synchronize variables that GLUT uses
  glui->sync_live();

  animation_frame = 0;
  if(animate_mode == 1) {
    // start animation
    GLUI_Master.set_glutIdleFunc(animate);
  } else {
    // stop animation
    GLUI_Master.set_glutIdleFunc(NULL);
  }
}

// Initialize GLUI and the user interface
void initGlui()
{
    GLUI_Master.set_glutIdleFunc(NULL);

    // Create GLUI window
    glui = GLUI_Master.create_glui("Glui Window", 0, Win[0]+10, 0);

    ///////////////////////////////////////////////////////////
    // TODO: 
    //   Add controls for additional joints here
    ///////////////////////////////////////////////////////////

    // Create a control to specify the rotation of the leg joint
    GLUI_Spinner *leg_joint_spinner
        = glui->add_spinner("Leg Joint", GLUI_SPINNER_FLOAT, &leg_rot);
    leg_joint_spinner->set_speed(0.1);
    leg_joint_spinner->set_float_limits(LEG_JOINT_MIN, LEG_JOINT_MAX, GLUI_LIMIT_CLAMP);    

    // Create a control to specify the rotation of the foot joint
    GLUI_Spinner *foot_joint_spinner
        = glui->add_spinner("Foot Joint", GLUI_SPINNER_FLOAT, &foot_rot);
    foot_joint_spinner->set_speed(0.1);
    foot_joint_spinner->set_float_limits(FOOT_JOINT_MIN, FOOT_JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Create a control to specify the rotation of the arm joint
    GLUI_Spinner *arm_joint_spinner
        = glui->add_spinner("Arm Joint", GLUI_SPINNER_FLOAT, &arm_rot);
    arm_joint_spinner->set_speed(0.1);
    arm_joint_spinner->set_float_limits(ARM_JOINT_MIN, ARM_JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Create a control to specify the rotation of the arm joint
    GLUI_Spinner *head_joint_spinner
        = glui->add_spinner("Head Joint", GLUI_SPINNER_FLOAT, &head_rot);
    head_joint_spinner->set_speed(0.1);
    head_joint_spinner->set_float_limits(HEAD_JOINT_MIN, HEAD_JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Create a control to specify the rotation of the arm joint
    GLUI_Spinner *beak_joint_spinner
        = glui->add_spinner("Beak Open/Close", GLUI_SPINNER_FLOAT, &beak_animate);
    beak_joint_spinner->set_speed(0.1);
    beak_joint_spinner->set_float_limits(BEAK_JOINT_MIN, BEAK_JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Create a control to specify the rotation of the arm joint
    GLUI_Spinner *xbody_joint_spinner
        = glui->add_spinner("X Body Animate", GLUI_SPINNER_FLOAT, &x_body_animate);
    xbody_joint_spinner->set_speed(0.1);
    xbody_joint_spinner->set_float_limits(X_BODY_JOINT_MIN, X_BODY_JOINT_MAX, GLUI_LIMIT_CLAMP);

    // Create a control to specify the rotation of the arm joint
    GLUI_Spinner *ybody_joint_spinner
        = glui->add_spinner("Y Body Animate", GLUI_SPINNER_FLOAT, &y_body_animate);
    ybody_joint_spinner->set_speed(0.1);
    ybody_joint_spinner->set_float_limits(Y_BODY_JOINT_MIN, Y_BODY_JOINT_MAX, GLUI_LIMIT_CLAMP);


    // Add button to specify animation mode 
    glui->add_separator();
    glui->add_checkbox("Animate", &animate_mode, 0, animateButton);

    // Add "Quit" button
    glui->add_separator();
    glui->add_button("Quit", 0, quitButton);

    // Set the main window to be the "active" window
    glui->set_main_gfx_window(windowID);
}


// Performs most of the OpenGL intialization
void initGl(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);
}


// Callback idle function for animating the scene
void animate()
{
    ///////////////////////////////////////////////////////////
    // TODO:
    //   Modify this function animate the character's joints
    //   Note: Nothing should be drawn in this function!  OpenGL drawing
    //   should only happen in the display() callback.
    ///////////////////////////////////////////////////////////

    // Update leg geometry
    const double leg_rot_speed = 0.15;
    double leg_rot_t = (sin(animation_frame*leg_rot_speed) + 1.0) / 2.0;
    leg_rot = leg_rot_t * LEG_JOINT_MIN + (1 - leg_rot_t) * LEG_JOINT_MAX;

    // Update foot geometry
    const double foot_rot_speed = 0.2;
    double foot_rot_t = (sin(animation_frame*foot_rot_speed) + 1.0) / 2.0;
    foot_rot = foot_rot_t * FOOT_JOINT_MIN + (1 - foot_rot_t) * FOOT_JOINT_MAX;

    // Update arm geometry
    const double arm_rot_speed = 0.15;
    double arm_rot_t = (sin(animation_frame*arm_rot_speed) + 1.0) / 2.0;
    arm_rot = arm_rot_t * ARM_JOINT_MIN + (1 - arm_rot_t) * ARM_JOINT_MAX;

    // Update head geometry
    const double head_rot_speed = 0.1;
    double head_rot_t = (sin(animation_frame*head_rot_speed) + 1.0) / 2.0;
    head_rot = head_rot_t * HEAD_JOINT_MIN + (1 - head_rot_t) * HEAD_JOINT_MAX;

    // Update beak geometry
    const double beak_animate_speed = 0.4;
    double beak_animate_t = (sin(animation_frame*beak_animate_speed) + 1.0) / 2.0;
    beak_animate = beak_animate_t * BEAK_JOINT_MIN + (1 - beak_animate_t) * BEAK_JOINT_MAX;

    // Update body geometry
    const double x_body_animate_speed = 0.005;
    const double y_body_animate_speed = 0.15;
    double x_frame_speed = animation_frame*x_body_animate_speed;    
    double y_frame_speed = (sin(animation_frame*y_body_animate_speed) + 1.0) / 2.0;
    y_body_animate = y_frame_speed * Y_BODY_JOINT_MIN + (1 - y_frame_speed) * Y_BODY_JOINT_MAX;

    // If the body reaches -350 on x, reset it back to 350, so the penguin
    //  can continually be animated
    if (x_body_animate <= -350.0) {
        x_body_animate = 350.0;
        animation_frame = 0.0;
    } else {
        x_body_animate = x_frame_speed * X_BODY_JOINT_MIN + (1 - x_frame_speed) * X_BODY_JOINT_MAX;
    }


    // Update user interface
    glui->sync_live();

    // Tell glut window to update itself.  This will cause the display()
    // callback to be called, which renders the object (once you've written
    // the callback).
    glutSetWindow(windowID);
    glutPostRedisplay();

    // increment the frame number.
    animation_frame++;

    // Wait 50 ms between frames (20 frames per second)
    usleep(50000);
}


// Handles the window being resized by updating the viewport
// and projection matrices
void myReshape(int w, int h)
{
    // Setup projection matrix for new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-w/2, w/2, -h/2, h/2);

    // Update OpenGL viewport and internal variables
    glViewport(0,0, w,h);
    Win[0] = w;
    Win[1] = h;
}


// display callback
//
// This gets called by the event handler to draw
// the scene, so this is where you need to build
// your scene -- make your changes and additions here.
// All rendering happens in this function.  For Assignment 1,
// updates to geometry should happen in the "animate" function.
void display(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);

    // OK, now clear the screen with the background colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

    // Setup the model-view transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ///////////////////////////////////////////////////////////
    // TODO:
    //   Modify this function draw the scene
    //   This should include function calls to pieces that
    //   apply the appropriate transformation matrice and
    //   render the individual body parts.
    ///////////////////////////////////////////////////////////

    // Declare the widths and lengths of the Penguin components

    const float JOINT_RADIUS = 4.0f;
    const float EYE_RADIUS = 5.0f;
    const float PUPIL_RADIUS = 2.0f;

    const float TORSO_X_VERTICES[] = { -1.0/4.0, 1.0/4.0, 1.0/2.0, 1.0/6.0, -1.0/6.0, -1.0/2.0 };
    const float TORSO_Y_VERTICES[] = { 1.0/2.0, 1.0/2.0, -3.0/10.0, -1.0/2.0, -1.0/2.0, -3.0/10.0 };
    const float TORSO_WIDTH = 125.0f;
    const float TORSO_LENGTH = 200.0f;

    const float LEG_X_VERTICES[] = { -1.0/2.0, 1.0/2.0, 1.0/2.0, -1.0/2.0 };
    const float LEG_Y_VERTICES[] = { 0.0, 0.0, -1.0, -1.0 };
    const float LEG_LENGTH = 60.0f;
    const float LEG_WIDTH = 16.0f;

    const float FOOT_X_VERTICES[] = { -1.0, 0.0, 0.0, -1.0 };
    const float FOOT_Y_VERTICES[] = { 1.0/2.0, 1.0/2.0, -1.0/2.0, -1.0/2.0 };
    const float FOOT_LENGTH = 13.0f;
    const float FOOT_WIDTH = 60.0f;

    const float ARM_X_VERTICES[] = { -1.0/2.0, 1.0/2.0, 1.0/4.0, -1.0/4.0 };
    const float ARM_Y_VERTICES[] = { 0.0, 0.0, -1.0, -1.0 };
    const float ARM_LENGTH = 80.0f;
    const float ARM_WIDTH = 40.0f;

    const float HEAD_X_VERTICES[] = { -3.0/8.0, -1.0/8.0, 1.0/4.0, 1.0/2.0, -1.0/2.0 };
    const float HEAD_Y_VERTICES[] = { 5.0/6.0, 1.0, 5.0/6.0, 0.0, 0.0 };
    const float HEAD_LENGTH = 55.0f;
    const float HEAD_WIDTH = 85.0f;

    const float BEAK_TOP_X_VERTICES[] = { -1.0/2.0, 1.0/2.0, 1.0/2.0, -1.0/2.0 };
    const float BEAK_TOP_Y_VERTICES[] = { 0.0, 1.0/2.0, -1.0/2.0, -1.0/2.0 };
    const float BEAK_BOTTOM_X_VERTICES[] = { 1.0/2.0, 1.0/2.0, -1.0/2.0, -1.0/2.0 };
    const float BEAK_BOTTOM_Y_VERTICES[] = { 1.0/2.0, -1.0/2.0, -1.0/2.0, 1.0/2.0 };    
    const float BEAK_TOP_LENGTH = 10.0f;
    const float BEAK_BOTTOM_LENGTH = 4.0f;
    const float BEAK_WIDTH = 45.0f;    


    // Push the current transformation matrix on the stack
    glPushMatrix();

        // Animated translation for body
        glTranslatef(x_body_animate, y_body_animate, 0.0);

        // Torso root of the kinematic tree
        glPushMatrix();
            glColor3f(0.0, 0.0, 1.0);


            // Draw torso
            //  - Push here so the torso scale doesnt affect other components
            glPushMatrix();
                glScalef(TORSO_WIDTH, TORSO_LENGTH, 1.0);
                drawPolygon(TORSO_X_VERTICES, TORSO_Y_VERTICES, 6);
            glPopMatrix();


            // Entire Right Leg leaf
            glPushMatrix();
                // Move hinge torso bottom left & draw hinge
                glTranslatef(TORSO_WIDTH/5.0, -TORSO_LENGTH/2.0 + LEG_LENGTH/2.0, 0.0);
                drawCircle(JOINT_RADIUS);
                // Rotate entire leg+foot by leg_rot angle (opposite of other leg)
                glRotatef(leg_rot, 0.0, 0.0, 1.0);

                // Push to adjust for hinge translate
                glPushMatrix();
                    // Translate a little above the hinge
                    glTranslatef(0.0, JOINT_RADIUS*2.0, 0.0);

                    // Draw right leg - Push here so the leg scale doesnt affect other components
                    glPushMatrix();
                        glScalef(LEG_WIDTH, LEG_LENGTH, 1.0);
                        drawPolygon(LEG_X_VERTICES, LEG_Y_VERTICES, 4);
                    glPopMatrix();

                    // Right foot
                    glPushMatrix();                    
                        // Move the hinge near bottom of the leg & draw
                        glTranslatef(0.0, -LEG_LENGTH + JOINT_RADIUS*(1.5), 0.0);
                        drawCircle(JOINT_RADIUS);

                        // This rotate applies to only the left foot
                        glRotatef(-foot_rot, 0.0, 0.0, 1.0);

                        // Push to adjust for hinge translate
                        glPushMatrix();
                            // Translate a little right of the hinge
                            glTranslatef(JOINT_RADIUS*2.0, 0.0, 0.0);    

                            // Draw left foot - Push here so the foot scale doesnt affect other components
                            glPushMatrix();
                                glScalef(FOOT_WIDTH, FOOT_LENGTH, 1.0);
                                drawPolygon(FOOT_X_VERTICES, FOOT_Y_VERTICES, 4);
                            glPopMatrix();
                        glPopMatrix();
                    glPopMatrix();
                glPopMatrix();
            glPopMatrix();


            // Entire Left Leg leaf
            glPushMatrix();
                // Move hinge torso bottom left & draw hinge
                glTranslatef(-TORSO_WIDTH/5.0, -TORSO_LENGTH/2.1 + LEG_LENGTH/2.0, 0.0);
                drawCircle(JOINT_RADIUS);
                // Rotate entire leg+foot by negative leg_rot angle (opposite of other leg)
                glRotatef(-leg_rot, 0.0, 0.0, 1.0);

                // Push to adjust for hinge translate
                glPushMatrix();
                    // Translate a little above the hinge
                    glTranslatef(0.0, JOINT_RADIUS*2.0, 0.0);

                    // Draw left leg - Push here so the leg scale doesnt affect other components
                    glPushMatrix();
                        glScalef(LEG_WIDTH, LEG_LENGTH, 1.0);
                        drawPolygon(LEG_X_VERTICES, LEG_Y_VERTICES, 4);
                    glPopMatrix();

                    // Left foot
                    glPushMatrix();                    
                        // Move the hinge near bottom of the leg & draw
                        glTranslatef(0.0, -LEG_LENGTH + JOINT_RADIUS*(1.5), 0.0);
                        drawCircle(JOINT_RADIUS);

                        // This rotate applies to only the left foot
                        glRotatef(-foot_rot, 0.0, 0.0, 1.0);

                        // Push to adjust for hinge translate
                        glPushMatrix();
                            // Translate a little right of the hinge
                            glTranslatef(JOINT_RADIUS*2.0, 0.0, 0.0);    

                            // Draw left foot - Push here so the foot scale doesnt affect other components
                            glPushMatrix();
                                glScalef(FOOT_WIDTH, FOOT_LENGTH, 1.0);
                                drawPolygon(FOOT_X_VERTICES, FOOT_Y_VERTICES, 4);
                            glPopMatrix();
                        glPopMatrix();
                    glPopMatrix();
                glPopMatrix();
            glPopMatrix();


            // Arm leaf
            glPushMatrix();
                // Want the hinge approx. 3/4 from the bottom of torso (1/2 from x axis), 
                //  and 1/12 of torso right of y axis
                glTranslatef(TORSO_WIDTH*(1.0/12.0), TORSO_LENGTH*(3.0/10.0), 0.0);
                // Draw hinge
                drawCircle(JOINT_RADIUS);
                // Rotate arm
                glRotatef(arm_rot, 0.0, 0.0, 1.0);

                // Draw arm - Push here so the arm scale doesnt affect other components
                glPushMatrix();
                    // Translate a little above the hinge
                    glTranslatef(0.0, JOINT_RADIUS*2.0, 0.0);
                    glScalef(ARM_WIDTH, ARM_LENGTH, 1.0);
                    drawPolygon(ARM_X_VERTICES, ARM_Y_VERTICES, 4);
                glPopMatrix();
            glPopMatrix();


            // Entire Head leaf
            glPushMatrix();
                // Move hinge to top of the torso, with a slighty overlap
                glTranslatef(0.0, TORSO_LENGTH/2.0 - HEAD_LENGTH/10.0, 0.0);
                // Draw hinge
                drawCircle(JOINT_RADIUS);
                // Rotate head
                glRotatef(head_rot, 0.0, 0.0, 1.0);

                // Push to adjust for hinge translate
                glPushMatrix();
                    // Translate a little below the hinge
                    glTranslatef(0.0, -JOINT_RADIUS*1.5, 0.0);

                    // Draw head - Push here so the head scale doesnt affect other components
                    glPushMatrix();
                        glScalef(HEAD_WIDTH, HEAD_LENGTH, 1.0);
                        drawPolygon(HEAD_X_VERTICES, HEAD_Y_VERTICES, 5);
                    glPopMatrix();

                    // Eye Leaf
                    glPushMatrix();
                        // Translate to upper left area of the head
                        glTranslatef(-HEAD_WIDTH*(1.0/4.0), HEAD_LENGTH*(2.0/3.0), 0.0);
                        // Draw eye & pupil (inner eye)
                        drawCircle(EYE_RADIUS);
                        drawFilledCircle(PUPIL_RADIUS);
                    glPopMatrix();

                    // Entire Beak Leaf
                    glPushMatrix();
                        // Translate for top beak
                        glTranslatef(-HEAD_WIDTH/2.0 - BEAK_WIDTH/3.5, HEAD_LENGTH/2.0, 0.0);

                        // Draw Top of Beak
                        glPushMatrix();
                            glScalef(BEAK_WIDTH, BEAK_TOP_LENGTH, 1.0);
                            drawPolygon(BEAK_TOP_X_VERTICES, BEAK_TOP_Y_VERTICES, 4);
                        glPopMatrix();

                        // Translation for bottom beak
                        glTranslatef(0.0, -BEAK_TOP_LENGTH*1.5, 0.0);
                        
                        glPushMatrix();
                            // Animated translation for bottom beak
                            glTranslatef(0.0, beak_animate, 0.0);

                            // Draw Bottom of Beak
                            glPushMatrix();
                                glScalef(BEAK_WIDTH, BEAK_BOTTOM_LENGTH, 1.0);
                                drawPolygon(BEAK_BOTTOM_X_VERTICES, BEAK_BOTTOM_Y_VERTICES, 4);
                            glPopMatrix();
                        glPopMatrix();
                    glPopMatrix();    
                glPopMatrix();
            glPopMatrix();
        glPopMatrix();

    // Retrieve the previous state of the transformation stack
    glPopMatrix();

    // Execute any GL functions that are in the queue just to be safe
    glFlush();

    // Now, show the frame buffer that we just drew into.
    // (this prevents flickering).
    glutSwapBuffers();
}



void drawCircle(float radius)
{
    glBegin( GL_LINE_LOOP );
        for( float angle = 0; angle < 2*PI; angle += delta_theta )
        glVertex2f( radius*cos(angle), radius*sin(angle));
    glEnd();
}

void drawFilledCircle(float radius)
{
    glBegin( GL_POLYGON );
        for( float angle = 0; angle < 2*PI; angle += delta_theta )
        glVertex2f( radius*cos(angle), radius*sin(angle));
    glEnd();
}


// Draw a rectangle of the relative specified size
void drawPolygon(const float xVertices[], const float yVertices[], int vCount)
{
    glBegin(GL_LINE_LOOP);
        for (int i = 0; i < vCount; i += 1) {
           glVertex2d(xVertices[i], yVertices[i]);
        }
    glEnd();
}