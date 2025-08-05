#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstdio>

// Enhanced camera and rotation system
struct Camera {
    float angleX = 25.0f;
    float angleY = -30.0f;
    float distance = 12.0f;
    float targetAngleX = 25.0f;
    float targetAngleY = -30.0f;
    bool smoothRotation = true;
};

// Face rotation animation
struct FaceRotation {
    int face = -1;          // 0=front, 1=back, 2=right, 3=left, 4=top, 5=bottom
    float angle = 0.0f;
    float targetAngle = 0.0f;
    bool isRotating = false;
    float speed = 5.0f;
};

// Cube state representation
int cubeState[6][9] = {
    {0,0,0,0,0,0,0,0,0}, // Front (Red)
    {1,1,1,1,1,1,1,1,1}, // Back (Orange)
    {2,2,2,2,2,2,2,2,2}, // Right (Green)
    {3,3,3,3,3,3,3,3,3}, // Left (Blue)
    {4,4,4,4,4,4,4,4,4}, // Top (White)
    {5,5,5,5,5,5,5,5,5}  // Bottom (Yellow)
};

Camera camera;
FaceRotation faceRotation;
bool autoRotate = false;
bool showHelp = false;
int windowWidth = 1000, windowHeight = 800;

// Enhanced color palette with better contrast
float colors[6][3] = {
    {0.9f, 0.1f, 0.1f},  // Red (Front)
    {1.0f, 0.6f, 0.0f},  // Orange (Back)
    {0.1f, 0.8f, 0.1f},  // Green (Right)
    {0.1f, 0.1f, 0.9f},  // Blue (Left)
    {0.95f, 0.95f, 0.95f}, // White (Top)
    {1.0f, 0.9f, 0.1f}   // Yellow (Bottom)
};

// Lighting setup
void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    
    // Main light
    float light0_pos[] = {5.0f, 5.0f, 5.0f, 1.0f};
    float light0_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    float light0_diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    float light0_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
    
    // Fill light
    float light1_pos[] = {-3.0f, -2.0f, 4.0f, 1.0f};
    float light1_diffuse[] = {0.4f, 0.4f, 0.4f, 1.0f};
    
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    
    // Material properties
    float mat_specular[] = {0.3f, 0.3f, 0.3f, 1.0f};
    float mat_shininess[] = {20.0f};
    
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

// Enhanced face drawing with better geometry
void drawColoredFace(int colorIndex, bool drawBorder = true) {
    float* color = colors[colorIndex];
    
    // Set material color
    float mat_diffuse[] = {color[0], color[1], color[2], 1.0f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    
    glColor3fv(color);
    
    // Draw main face with slight inset for better visual separation
    float inset = 0.05f;
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-0.5f + inset, -0.5f + inset, 0.501f);
    glVertex3f( 0.5f - inset, -0.5f + inset, 0.501f);
    glVertex3f( 0.5f - inset,  0.5f - inset, 0.501f);
    glVertex3f(-0.5f + inset,  0.5f - inset, 0.501f);
    glEnd();
    
    // Draw border
    if (drawBorder) {
        glDisable(GL_LIGHTING);
        glColor3f(0.0f, 0.0f, 0.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex3f(-0.5f + inset, -0.5f + inset, 0.502f);
        glVertex3f( 0.5f - inset, -0.5f + inset, 0.502f);
        glVertex3f( 0.5f - inset,  0.5f - inset, 0.502f);
        glVertex3f(-0.5f + inset,  0.5f - inset, 0.502f);
        glEnd();
        glEnable(GL_LIGHTING);
    }
}

// Enhanced cubie with proper face mapping
void drawCubie(int x, int y, int z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    
    // Draw black outline
    glDisable(GL_LIGHTING);
    glColor3f(0.05f, 0.05f, 0.05f);
    glLineWidth(1.5f);
    glutWireCube(1.02f);
    glEnable(GL_LIGHTING);
    
    float offset = 0.501f;
    
    // Front face (Z = 1) - Red
    if (z == 1) {
        glPushMatrix();
        glTranslatef(0, 0, offset);
        // Map 3D position to 2D face grid: top-left is (0,0), bottom-right is (2,2)
        int row = (1 - y);  // y=1 -> row=0 (top), y=-1 -> row=2 (bottom)
        int col = (x + 1);  // x=-1 -> col=0 (left), x=1 -> col=2 (right)
        int faceIndex = row * 3 + col;
        drawColoredFace(cubeState[0][faceIndex]);
        glPopMatrix();
    }
    
    // Back face (Z = -1) - Orange
    if (z == -1) {
        glPushMatrix();
        glTranslatef(0, 0, -offset);
        glRotatef(180, 0, 1, 0);
        int row = (1 - y);
        int col = (1 - x);  // Flipped because we're looking from behind
        int faceIndex = row * 3 + col;
        drawColoredFace(cubeState[1][faceIndex]);
        glPopMatrix();
    }
    
    // Top face (Y = 1) - White
    if (y == 1) {
        glPushMatrix();
        glTranslatef(0, offset, 0);
        glRotatef(-90, 1, 0, 0);
        int row = (1 - z);  // z=1 -> row=0 (front), z=-1 -> row=2 (back)
        int col = (x + 1);
        int faceIndex = row * 3 + col;
        drawColoredFace(cubeState[4][faceIndex]);
        glPopMatrix();
    }
    
    // Bottom face (Y = -1) - Yellow
    if (y == -1) {
        glPushMatrix();
        glTranslatef(0, -offset, 0);
        glRotatef(90, 1, 0, 0);
        int row = (z + 1);  // z=-1 -> row=0 (back), z=1 -> row=2 (front)
        int col = (x + 1);
        int faceIndex = row * 3 + col;
        drawColoredFace(cubeState[5][faceIndex]);
        glPopMatrix();
    }
    
    // Left face (X = -1) - Blue
    if (x == -1) {
        glPushMatrix();
        glTranslatef(-offset, 0, 0);
        glRotatef(90, 0, 1, 0);
        int row = (1 - y);
        int col = (z + 1);  // z=-1 -> col=0 (back), z=1 -> col=2 (front)
        int faceIndex = row * 3 + col;
        drawColoredFace(cubeState[3][faceIndex]);
        glPopMatrix();
    }
    
    // Right face (X = 1) - Green
    if (x == 1) {
        glPushMatrix();
        glTranslatef(offset, 0, 0);
        glRotatef(-90, 0, 1, 0);
        int row = (1 - y);
        int col = (1 - z);  // z=1 -> col=0 (front), z=-1 -> col=2 (back)
        int faceIndex = row * 3 + col;
        drawColoredFace(cubeState[2][faceIndex]);
        glPopMatrix();
    }
    
    glPopMatrix();
}

// Enhanced cube drawing with face rotation support
void drawRubiksCube() {
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            for (int z = -1; z <= 1; z++) {
                bool needsRotation = false;
                
                // Check if this cubie is part of the rotating face
                if (faceRotation.isRotating) {
                    switch (faceRotation.face) {
                        case 0: needsRotation = (z == 1); break;  // Front
                        case 1: needsRotation = (z == -1); break; // Back
                        case 2: needsRotation = (x == 1); break;  // Right
                        case 3: needsRotation = (x == -1); break; // Left
                        case 4: needsRotation = (y == 1); break;  // Top
                        case 5: needsRotation = (y == -1); break; // Bottom
                    }
                }
                
                if (needsRotation) {
                    glPushMatrix();
                    // Apply rotation around appropriate axis
                    switch (faceRotation.face) {
                        case 0: case 1: // Front/Back - rotate around Z
                            glRotatef(faceRotation.angle, 0, 0, faceRotation.face == 0 ? 1 : -1);
                            break;
                        case 2: case 3: // Right/Left - rotate around X
                            glRotatef(faceRotation.angle, faceRotation.face == 2 ? 1 : -1, 0, 0);
                            break;
                        case 4: case 5: // Top/Bottom - rotate around Y
                            glRotatef(faceRotation.angle, 0, faceRotation.face == 4 ? 1 : -1, 0);
                            break;
                    }
                    drawCubie(x, y, z);
                    glPopMatrix();
                } else {
                    drawCubie(x, y, z);
                }
            }
        }
    }
}

// Rotate a face 90 degrees clockwise in the state array
void rotateFaceClockwise(int face) {
    int temp[9];
    // Copy current state
    for (int i = 0; i < 9; i++) {
        temp[i] = cubeState[face][i];
    }
    
    // Rotate clockwise: map old positions to new positions
    // 0 1 2    6 3 0
    // 3 4 5 -> 7 4 1
    // 6 7 8    8 5 2
    cubeState[face][0] = temp[6];
    cubeState[face][1] = temp[3];
    cubeState[face][2] = temp[0];
    cubeState[face][3] = temp[7];
    cubeState[face][4] = temp[4]; // center stays
    cubeState[face][5] = temp[1];
    cubeState[face][6] = temp[8];
    cubeState[face][7] = temp[5];
    cubeState[face][8] = temp[2];
}

// Rotate adjacent edges when a face is rotated
void rotateAdjacentEdges(int face) {
    int temp[3];
    
    switch (face) {
        case 0: // Front face - affects Top, Right, Bottom, Left edges
            // Save top edge
            temp[0] = cubeState[4][6]; temp[1] = cubeState[4][7]; temp[2] = cubeState[4][8];
            // Top <- Left
            cubeState[4][6] = cubeState[3][8]; cubeState[4][7] = cubeState[3][5]; cubeState[4][8] = cubeState[3][2];
            // Left <- Bottom
            cubeState[3][2] = cubeState[5][2]; cubeState[3][5] = cubeState[5][1]; cubeState[3][8] = cubeState[5][0];
            // Bottom <- Right
            cubeState[5][0] = cubeState[2][6]; cubeState[5][1] = cubeState[2][3]; cubeState[5][2] = cubeState[2][0];
            // Right <- Top (from temp)
            cubeState[2][0] = temp[0]; cubeState[2][3] = temp[1]; cubeState[2][6] = temp[2];
            break;
            
        case 1: // Back face - affects Top, Left, Bottom, Right edges
            // Save top edge
            temp[0] = cubeState[4][0]; temp[1] = cubeState[4][1]; temp[2] = cubeState[4][2];
            // Top <- Right
            cubeState[4][0] = cubeState[2][2]; cubeState[4][1] = cubeState[2][5]; cubeState[4][2] = cubeState[2][8];
            // Right <- Bottom
            cubeState[2][2] = cubeState[5][8]; cubeState[2][5] = cubeState[5][7]; cubeState[2][8] = cubeState[5][6];
            // Bottom <- Left
            cubeState[5][6] = cubeState[3][0]; cubeState[5][7] = cubeState[3][3]; cubeState[5][8] = cubeState[3][6];
            // Left <- Top (from temp)
            cubeState[3][0] = temp[2]; cubeState[3][3] = temp[1]; cubeState[3][6] = temp[0];
            break;
            
        case 2: // Right face - affects Top, Back, Bottom, Front edges
            // Save top edge
            temp[0] = cubeState[4][2]; temp[1] = cubeState[4][5]; temp[2] = cubeState[4][8];
            // Top <- Front
            cubeState[4][2] = cubeState[0][2]; cubeState[4][5] = cubeState[0][5]; cubeState[4][8] = cubeState[0][8];
            // Front <- Bottom
            cubeState[0][2] = cubeState[5][2]; cubeState[0][5] = cubeState[5][5]; cubeState[0][8] = cubeState[5][8];
            // Bottom <- Back
            cubeState[5][2] = cubeState[1][6]; cubeState[5][5] = cubeState[1][3]; cubeState[5][8] = cubeState[1][0];
            // Back <- Top (from temp)
            cubeState[1][0] = temp[2]; cubeState[1][3] = temp[1]; cubeState[1][6] = temp[0];
            break;
            
        case 3: // Left face - affects Top, Front, Bottom, Back edges
            // Save top edge
            temp[0] = cubeState[4][0]; temp[1] = cubeState[4][3]; temp[2] = cubeState[4][6];
            // Top <- Back
            cubeState[4][0] = cubeState[1][8]; cubeState[4][3] = cubeState[1][5]; cubeState[4][6] = cubeState[1][2];
            // Back <- Bottom
            cubeState[1][2] = cubeState[5][6]; cubeState[1][5] = cubeState[5][3]; cubeState[1][8] = cubeState[5][0];
            // Bottom <- Front
            cubeState[5][0] = cubeState[0][0]; cubeState[5][3] = cubeState[0][3]; cubeState[5][6] = cubeState[0][6];
            // Front <- Top (from temp)
            cubeState[0][0] = temp[0]; cubeState[0][3] = temp[1]; cubeState[0][6] = temp[2];
            break;
            
        case 4: // Top face - affects Front, Right, Back, Left edges
            // Save front edge
            temp[0] = cubeState[0][0]; temp[1] = cubeState[0][1]; temp[2] = cubeState[0][2];
            // Front <- Right
            cubeState[0][0] = cubeState[2][0]; cubeState[0][1] = cubeState[2][1]; cubeState[0][2] = cubeState[2][2];
            // Right <- Back
            cubeState[2][0] = cubeState[1][0]; cubeState[2][1] = cubeState[1][1]; cubeState[2][2] = cubeState[1][2];
            // Back <- Left
            cubeState[1][0] = cubeState[3][0]; cubeState[1][1] = cubeState[3][1]; cubeState[1][2] = cubeState[3][2];
            // Left <- Front (from temp)
            cubeState[3][0] = temp[0]; cubeState[3][1] = temp[1]; cubeState[3][2] = temp[2];
            break;
            
        case 5: // Bottom face - affects Front, Left, Back, Right edges
            // Save front edge
            temp[0] = cubeState[0][6]; temp[1] = cubeState[0][7]; temp[2] = cubeState[0][8];
            // Front <- Left
            cubeState[0][6] = cubeState[3][6]; cubeState[0][7] = cubeState[3][7]; cubeState[0][8] = cubeState[3][8];
            // Left <- Back
            cubeState[3][6] = cubeState[1][6]; cubeState[3][7] = cubeState[1][7]; cubeState[3][8] = cubeState[1][8];
            // Back <- Right
            cubeState[1][6] = cubeState[2][6]; cubeState[1][7] = cubeState[2][7]; cubeState[1][8] = cubeState[2][8];
            // Right <- Front (from temp)
            cubeState[2][6] = temp[0]; cubeState[2][7] = temp[1]; cubeState[2][8] = temp[2];
            break;
    }
}

// Perform a complete face rotation (both face and adjacent edges)
void performFaceRotation(int face) {
    rotateFaceClockwise(face);
    rotateAdjacentEdges(face);
}
    srand(time(nullptr));
    for (int i = 0; i < 50; i++) {
        // Randomly swap colors between faces
        int face1 = rand() % 6;
        int face2 = rand() % 6;
        int pos1 = rand() % 9;
        int pos2 = rand() % 9;
        
        int temp = cubeState[face1][pos1];
        cubeState[face1][pos1] = cubeState[face2][pos2];
        cubeState[face2][pos2] = temp;
    }
}

// Reset cube to solved state
void resetCube() {
    for (int face = 0; face < 6; face++) {
        for (int pos = 0; pos < 9; pos++) {
            cubeState[face][pos] = face;
        }
    }
}

// Render text
void renderText(float x, float y, const char* text) {
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *text);
        text++;
    }
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);
}

// Display help overlay
void displayHelp() {
    if (!showHelp) return;
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Semi-transparent background
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(windowWidth, 0);
    glVertex2f(windowWidth, windowHeight);
    glVertex2f(0, windowHeight);
    glEnd();
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    // Help text
    renderText(50, 50, "ENHANCED RUBIK'S CUBE CONTROLS:");
    renderText(50, 80, "WASD - Rotate camera");
    renderText(50, 100, "Mouse - Interactive camera control");
    renderText(50, 120, "R - Reset cube to solved state");
    renderText(50, 140, "Space - Scramble cube");
    renderText(50, 160, "A - Toggle auto-rotation");
    renderText(50, 180, "123456 - Rotate faces (Front/Back/Right/Left/Top/Bottom)");
    renderText(50, 200, "+ / - - Zoom in/out");
    renderText(50, 220, "H - Toggle this help");
    renderText(50, 240, "ESC - Exit");
    renderText(50, 280, "Press H again to close help");
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

// Enhanced display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Smooth camera interpolation
    if (camera.smoothRotation) {
        camera.angleX += (camera.targetAngleX - camera.angleX) * 0.1f;
        camera.angleY += (camera.targetAngleY - camera.angleY) * 0.1f;
    }
    
    // Auto rotation
    if (autoRotate) {
        camera.targetAngleY += 0.5f;
    }
    
    // Position camera
    glTranslatef(0.0f, 0.0f, -camera.distance);
    glRotatef(camera.angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(camera.angleY, 0.0f, 1.0f, 0.0f);
    
    // Update face rotation animation
    if (faceRotation.isRotating) {
        float diff = faceRotation.targetAngle - faceRotation.angle;
        if (fabs(diff) > 0.1f) {
            faceRotation.angle += diff * 0.15f;
        } else {
            faceRotation.angle = faceRotation.targetAngle;
            faceRotation.isRotating = false;
        }
        glutPostRedisplay();
    }
    
    setupLighting();
    drawRubiksCube();
    
    displayHelp();
    
    glutSwapBuffers();
}

// Enhanced reshape function
void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    
    if (h == 0) h = 1;
    float ratio = 1.0f * w / h;
    
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, ratio, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// Mouse interaction
int lastMouseX = 0, lastMouseY = 0;
bool mousePressed = false;

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        mousePressed = (state == GLUT_DOWN);
        lastMouseX = x;
        lastMouseY = y;
    }
}

void mouseMotion(int x, int y) {
    if (mousePressed) {
        float deltaX = x - lastMouseX;
        float deltaY = y - lastMouseY;
        
        camera.targetAngleY += deltaX * 0.5f;
        camera.targetAngleX += deltaY * 0.5f;
        
        // Clamp X rotation
        if (camera.targetAngleX > 90.0f) camera.targetAngleX = 90.0f;
        if (camera.targetAngleX < -90.0f) camera.targetAngleX = -90.0f;
        
        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();
    }
}

// Enhanced keyboard controls
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        // Camera controls
        case 'w': case 'W':
            camera.targetAngleX -= 5.0f;
            break;
        case 's': case 'S':
            camera.targetAngleX += 5.0f;
            break;
        case 'a': case 'A':
            if (key == 'a') camera.targetAngleY -= 5.0f;
            else autoRotate = !autoRotate;
            break;
        case 'd': case 'D':
            camera.targetAngleY += 5.0f;
            break;
            
        // Face rotations
        case '1': // Front face
            if (!faceRotation.isRotating) {
                faceRotation.face = 0;
                faceRotation.targetAngle += 90.0f;
                faceRotation.isRotating = true;
                performFaceRotation(0); // Update cube state immediately
            }
            break;
        case '2': // Back face
            if (!faceRotation.isRotating) {
                faceRotation.face = 1;
                faceRotation.targetAngle += 90.0f;
                faceRotation.isRotating = true;
                performFaceRotation(1);
            }
            break;
        case '3': // Right face
            if (!faceRotation.isRotating) {
                faceRotation.face = 2;
                faceRotation.targetAngle += 90.0f;
                faceRotation.isRotating = true;
                performFaceRotation(2);
            }
            break;
        case '4': // Left face
            if (!faceRotation.isRotating) {
                faceRotation.face = 3;
                faceRotation.targetAngle += 90.0f;
                faceRotation.isRotating = true;
                performFaceRotation(3);
            }
            break;
        case '5': // Top face
            if (!faceRotation.isRotating) {
                faceRotation.face = 4;
                faceRotation.targetAngle += 90.0f;
                faceRotation.isRotating = true;
                performFaceRotation(4);
            }
            break;
        case '6': // Bottom face
            if (!faceRotation.isRotating) {
                faceRotation.face = 5;
                faceRotation.targetAngle += 90.0f;
                faceRotation.isRotating = true;
                performFaceRotation(5);
            }
            break;
            
        // Utility controls
        case ' ': // Spacebar - scramble
            scrambleCube();
            break;
        case 'r': case 'R': // Reset
            resetCube();
            break;
        case 'h': case 'H': // Help
            showHelp = !showHelp;
            break;
        case '+': case '=': // Zoom in
            camera.distance = fmax(5.0f, camera.distance - 1.0f);
            break;
        case '-': case '_': // Zoom out
            camera.distance = fmin(20.0f, camera.distance + 1.0f);
            break;
        case 27: // ESC
            exit(0);
            break;
    }
    glutPostRedisplay();
}

// Idle function for smooth animations
void idle() {
    if (autoRotate || faceRotation.isRotating || 
        fabs(camera.targetAngleX - camera.angleX) > 0.1f ||
        fabs(camera.targetAngleY - camera.angleY) > 0.1f) {
        glutPostRedisplay();
    }
}

// Enhanced initialization
void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);
    
    // Enhanced background gradient effect
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    
    // Anti-aliasing
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    resetCube();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Enhanced Interactive Rubik's Cube");
    
    init();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutIdleFunc(idle);
    
    printf("Enhanced Rubik's Cube Controls:\n");
    printf("WASD - Camera rotation, Mouse - Interactive control\n");
    printf("1-6 - Rotate faces, Space - Scramble, R - Reset\n");
    printf("H - Help overlay, A - Auto-rotate, +/- - Zoom\n");
    printf("ESC - Exit\n\n");
    
    glutMainLoop();
    return 0;
}