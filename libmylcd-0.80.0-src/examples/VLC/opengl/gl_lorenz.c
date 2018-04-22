/*
 * Lorenz Attractor Demo
 *
 * Adapted from code originally written for the 4D60GT by
 * Aaron T. Ferrucci (aaronf@cse.ucsc.edu), 7/3/92.
 *
 * Description:
 *
 * This program shows some particles stuck in a Lorenz attractor (the parameters
 * used are r=28, b=8/3, sigma=10). The eye is attracted to the red particle,
 * with a force directly proportionate to distance. A command line
 * puts the whole mess inside a box made of hexagons. I think this helps to
 * maintain the illusion of 3 dimensions, but it can slow things down.
 * Other options allow you to play with the redraw rate and the number of new
 * lines per redraw. So you can customize it to the speed of your machine.
 * 
 * For general info on Lorenz attractors I recommend "An Introduction to
 * the Lorenz Equations", IEEE Transactions on Circuits and Systems, August '83.
 *
 * Bugs: hidden surface removal doesn't apply to hexagons, and
 * works poorly on lines when they are too close together.
 *
 * Notes on OpenGL port:
 * 
 * The timer functions do not exist in OpenGL, so the drawing occurs in a
 * continuous loop, controlled by step, stop and go input from the keyboard.
 * Perhaps system function could be called to control timing.
 *
 */



#include "../common.h"
#include "mylcdgl.h"


#ifdef WIN32
#define drand48() ((float)rand()/RAND_MAX)
#define srand48(x) (srand((x)))
#endif


#define POINTMASK (unsigned long)511
#define G (0.002)	/* eyept to red sphere gravity */
#define LG (0.3)
#define CUBESIDE (120.0)
#define CUBESCALE (23.0)
#define CUBEOFFX (-4.0)
#define CUBEOFFY (0.0)
#define CUBEOFFZ (57.0)
#define FALSE 0
#define TRUE 1

/* globals */
static float sigma = 10.0, r = 28.0, b = 8./3.0, dt = 0.003;
static unsigned long rp = 0, bp = 0, gp = 0, yp = 0, mp = 0;
static long xmax, ymax;
static float rv[POINTMASK+1][3],			/* red points */
	bv[POINTMASK+1][3],		/* blue points */
	gv[POINTMASK+1][3],		/* green points */
	yv[POINTMASK+1][3],		/* yellow points */
	mv[POINTMASK+1][3];		/* magenta points */

static float eyex[3],	/* eye location */
	 eyev[3],	/* eye velocity */
	 eyel[3];	/* lookat point location */
static GLint fovy = 600;
static float dx, dy, dz;
static GLUquadricObj *quadObj;
static GLuint asphere = 0;
static float cubeoffx = CUBEOFFX;
static float cubeoffy = CUBEOFFY;
static float cubeoffz = CUBEOFFZ;
static float farplane = 100.0;

static int animate = 1;

/* option flags */
static GLboolean hexflag,		/* hexagons? */
	wflag,
	gflag;

/* option values */
static int speed;		/* speed (number of new line segs per redraw) */
static float a = 0,
    da;			/* hexagon rotational velocity (.1 degree/redraw) */
static float gravity;


/* function declarations */
void init_3d (void);
void init_graphics (const int width, const int height);
void draw_hexcube (void);
void draw_hexplane (void);
void draw_hexagon (void);
static void move_eye (void);
static void redraw (void);
static void next_line (float v[][3], unsigned long *p);
static void sphdraw (const float args[4]);
void setPerspective (const int angle, const float aspect, const float zNear, const float zFar);
void setDefaultFlags ();

static void Reshape (const int width, const int height)
{
	glViewport(0,0,width,height);
	glClear(GL_COLOR_BUFFER_BIT);
	xmax = width;
	ymax = height;
}


static void Draw (void)
{
    int i;

    if (animate) {
		i = speed;
		
		while (i--) {
	    	next_line(rv, &rp);
	    	next_line(bv, &bp);
	    	next_line(gv, &gp);
	    	next_line(yv, &yp);
	    	next_line(mv, &mp);
		}
		glPushMatrix();
		move_eye();
		redraw();
		glPopMatrix();
    }
}

int GLLorenzInit (TFRAME *frame)
{
    setDefaultFlags();
    mylcdgl_init(frame, MYGL_CREATECONTEXT);
	//mylcdgl_draw(frame, MYGL_CREATECONTEXT/* | MYGL_SHOWWINDOW*/);
    init_3d();
    init_graphics(frame->width, frame->height);
	Reshape(frame->width, frame->height);


    /* draw the first POINTMASK points in each color */
    while(rp < POINTMASK){
		next_line(rv, &rp);
		next_line(bv, &bp);
		next_line(gv, &gp);
		next_line(yv, &yp);
		next_line(mv, &mp);
    }

    eyex[0] = eyex[1] = eyex[2] = 0.0;
    eyel[0] = rv[rp][0];
    eyel[1] = rv[rp][1];
    eyel[2] = rv[rp][2];
	
    glPushMatrix();
    move_eye();
    redraw();
    glPopMatrix();
    return 1;
}

void GLLorenzDraw (TFRAME *frame)
{
	Draw();
	mylcdgl_draw(frame, MYGL_FRONTPIXELSONLY);
}

void GLLorenzClose ()
{
	animate = 0;
	gluDeleteQuadric(quadObj);
	glDeleteLists(asphere, 1);
	asphere = 0;
	quadObj = NULL;
	mylcdgl_shutdown();
}

/* compute the next point on the path according to Lorenz' equations. */
static void next_line (float v[][3], unsigned long *p)
{

    dx = sigma * (v[*p][1] - v[*p][0]) * dt;
    dy = (r*v[*p][0] - v[*p][1] + v[*p][0]*v[*p][2]) * dt;
    dz = (v[*p][0] *v[*p][1] + b*v[*p][2]) * dt;	
	
    v[(*p + 1) & POINTMASK][0] = v[*p][0] + dx;
    v[(*p + 1) & POINTMASK][1] = v[*p][1] + dy;
    v[(*p + 1) & POINTMASK][2] = v[*p][2] - dz;
    *p = (*p + 1) & POINTMASK;
}

static void drawLines (const unsigned long index, float array[POINTMASK][3])
{
#define LINE_STEP 4

    unsigned long p = (index+1)&POINTMASK;
    int i = LINE_STEP-(p % LINE_STEP);
    
    if (i == LINE_STEP) i = 0;
    
    glBegin(GL_LINE_STRIP);
	/* draw points in order from oldest to newest */
	while(p != index) {
	    if (i == 0) {
			glVertex3fv(array[p]);
			i = LINE_STEP;
	    } 
	    i--;
	    p = (p+1) & POINTMASK;
	}
	glVertex3fv(array[index]);
    glEnd();
}

static void redraw (void)
{
	
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (hexflag)
		draw_hexcube();

    glColor3f(1.0, 0.0, 0.0);
    drawLines(rp, rv);
    sphdraw(rv[rp]);

    glColor3f(0.0, 0.0, 1.0);
    drawLines(bp, bv);
    sphdraw(bv[bp]);
	
    glColor3f(0.0, 1.0, 0.0);
    drawLines(gp, gv);
    sphdraw(gv[gp]);

    glColor3f(1.0, 0.0, 1.0);
    drawLines(yp, yv);
    sphdraw(yv[yp]);

    glColor3f(0.0, 1.0, 1.0);
    drawLines(mp, mv);
    sphdraw(mv[mp]);
}

static void move_eye (void)
{
    /* first move the eye */
    eyev[0] += gravity * (rv[rp][0] - eyex[0]);
    eyev[1] += gravity * (rv[rp][1] - eyex[1]);
    eyev[2] += gravity * (rv[rp][2] - eyex[2]);

    /* adjust position using new velocity */
    eyex[0] += eyev[0] * dt;
    eyex[1] += eyev[1] * dt;
    eyex[2] += eyev[2] * dt;

    /* move the lookat point */
    /* it catches up to the red point if it's moving slowly enough */
    eyel[0] += LG * (rv[rp][0] - eyel[0]);
    eyel[1] += LG * (rv[rp][1] - eyel[1]);
    eyel[2] += LG * (rv[rp][2] - eyel[2]);

    /* change view */
    gluLookAt(eyex[0], eyex[1], eyex[2], eyel[0], eyel[1], eyel[2],
	      0, 1, 0);
}

void draw_hexcube(void)
{

    a += da;
    if (a >= 720.0)		/* depends on slowest rotation factor */
		a = 0.0;

    /* draw hexplanes, without changing z-values */
    glDepthMask(GL_FALSE); 
    glDisable(GL_DEPTH_TEST);

    /* x-y plane */
    glColor3f(0.2, 0.2, 0.6);
    glPushMatrix();
    glTranslatef(cubeoffx, cubeoffy, cubeoffz);
    glScalef(CUBESCALE, CUBESCALE, CUBESCALE);
    draw_hexplane();
    glPopMatrix();

    /* x-y plane, translated */
    glPushMatrix();
    glTranslatef(cubeoffx, cubeoffy, cubeoffz - 2*CUBESIDE);
    glScalef(CUBESCALE, CUBESCALE, CUBESCALE);
    draw_hexplane();
    glPopMatrix();

    glColor3f(0.6, 0.2, 0.2);
    /* x-z plane, translate low */
    glPushMatrix();
    glRotatef(90, 1.0, 0.0, 0.0);
    glTranslatef(cubeoffx, cubeoffz - CUBESIDE, -cubeoffy + CUBESIDE);
    glScalef(CUBESCALE, CUBESCALE, CUBESCALE);
    draw_hexplane();
    glPopMatrix();

    /* x-z plane, translate high */
    glPushMatrix();
    glRotatef(90, 1.0, 0.0, 0.0);
    glTranslatef(cubeoffx, cubeoffz - CUBESIDE, -cubeoffy - CUBESIDE);
    glScalef(CUBESCALE, CUBESCALE, CUBESCALE);
    draw_hexplane();
    glPopMatrix();

    glColor3f(0.2, 0.6, 0.2);
    /* y-z plane, translate low */
    glPushMatrix();
    glRotatef(90, 0.0, 1.0, 0.0);
    glTranslatef(-cubeoffz + CUBESIDE, cubeoffy, cubeoffx + CUBESIDE);
    glScalef(CUBESCALE, CUBESCALE, CUBESCALE);
    draw_hexplane();
    glPopMatrix();
	
    /* y-z plane, translate high */
    glPushMatrix();
    glRotatef (90, 0.0, 1.0, 0.0);
    glTranslatef(-cubeoffz + CUBESIDE, cubeoffy, cubeoffx - CUBESIDE);
    glScalef(CUBESCALE, CUBESCALE, CUBESCALE);
    draw_hexplane();
    glPopMatrix();

    glFlush();
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

float hex_data[8][3] =  {
    {0.0, 0.0, 0.0},
    {1.155, 0.0, 0.0},
    {0.577, 1.0, 0.0},
    {-0.577, 1.0, 0.0},
    {-1.155, 0.0, 0.0},
    {-0.577, -1.0, 0.0},
    {0.577, -1.0, 0.0},
    {1.155, 0.0, 0.0},
};

/* draws a hexagon 2 units across, in the x-y plane, */
/* centered at <0, 0, 0> */

void draw_hexagon(void)
{
    if (wflag){
		glPushMatrix();
		glRotatef(a, 0.0, 0.0, 1.0);
    }

    glBegin(GL_TRIANGLE_FAN);
	glVertex3fv(hex_data[0]);
	glVertex3fv(hex_data[1]);
	glVertex3fv(hex_data[2]);
	glVertex3fv(hex_data[3]);
	glVertex3fv(hex_data[4]);
	glVertex3fv(hex_data[5]);
	glVertex3fv(hex_data[6]);
	glVertex3fv(hex_data[7]);
    glEnd();

    if (wflag)
		glPopMatrix();
}

void tmp_draw_hexplane(void)
{
    glRectf(-2.0, -2.0, 2.0, 2.0);
}

/* draw 7 hexagons */
void draw_hexplane(void)
{
    if (wflag){
		glPushMatrix();
		glRotatef(-0.5*a, 0.0, 0.0, 1.0);
    }

    /* center , <0, 0, 0> */
    draw_hexagon();

    /* 12 o'clock, <0, 4, 0> */
    glTranslatef(0.0, 4.0, 0.0);
    draw_hexagon();

    /* 10 o'clock, <-3.464, 2, 0> */
    glTranslatef(-3.464, -2.0, 0.0);
    draw_hexagon();

    /* 8 o'clock, <-3.464, -2, 0> */
    glTranslatef(0.0, -4.0, 0.0);
    draw_hexagon();

    /* 6 o'clock, <0, -4, 0> */
    glTranslatef(3.464, -2.0, 0.0);
    draw_hexagon();

    /* 4 o'clock, <3.464, -2, 0> */
    glTranslatef(3.464, 2.0, 0.0);
    draw_hexagon();

    /* 2 o'clock, <3.464, 2, 0> */
    glTranslatef(0.0, 4.0, 0.0);
    draw_hexagon();

    if (wflag)
		glPopMatrix();
}

static void sphdraw (const float args[3])
{
    glPushMatrix();
    glTranslatef(args[0], args[1], args[2]);
    glCallList(asphere);
    glPopMatrix();
}

void setPerspective (const int angle, const float aspect, const float zNear, const float zFar)
{
    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_PROJECTION);
    gluPerspective(angle * 0.1, aspect, zNear, zFar);
    glPopAttrib();
}

/* initialize global 3-vectors */
void init_3d (void)
{
    (void)srand48((long)time((time_t*)NULL));

    /* initialize colored points */
    rv[0][0] = (float)drand48() * 10.0;
    rv[0][1] = (float)drand48() * 10.0;
    rv[0][2] = (float)drand48() * 10.0 - 10.0;

    bv[0][0] = rv[0][0] + (float)drand48()*5.0;
    bv[0][1] = rv[0][1] + (float)drand48()*5.0;
    bv[0][0] = rv[0][2] + (float)drand48()*5.0;

    gv[0][0] = rv[0][0] + (float)drand48()*5.0;
    gv[0][1] = rv[0][1] + (float)drand48()*5.0;
    gv[0][0] = rv[0][2] + (float)drand48()*5.0;

    yv[0][0] = rv[0][0] + (float)drand48()*5.0;
    yv[0][1] = rv[0][1] + (float)drand48()*5.0;
    yv[0][0] = rv[0][2] + (float)drand48()*5.0;

    mv[0][0] = rv[0][0] + (float)drand48()*5.0;
    mv[0][1] = rv[0][1] + (float)drand48()*5.0;
    mv[0][0] = rv[0][2] + (float)drand48()*5.0;

    /* initialize eye velocity */
    eyev[0] = eyev[1] = eyev[2] = 0.0;
}


void init_graphics (const int width, const int height)
{
    xmax = width;
    ymax = height;
    
    glDrawBuffer(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(1.0);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glViewport(0, 0, xmax, ymax);
    setPerspective(fovy, (float)xmax/(float)ymax, 0.01, farplane);
    quadObj = gluNewQuadric();
    gluQuadricNormals(quadObj, GLU_NONE);
    asphere = glGenLists(1);
    glNewList(asphere, GL_COMPILE);
    gluSphere(quadObj, 0.3, 12, 8);
    glEndList();
}

void setDefaultFlags ()
{
	animate = 1;
    hexflag = FALSE;
    wflag = FALSE;
    gflag = FALSE;

	speed = 3;
		
    if (!wflag)
		da = 0.0;
		
    if (!gflag)
		gravity = G;
}
