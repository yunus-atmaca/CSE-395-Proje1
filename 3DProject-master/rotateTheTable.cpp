//
// Created by mpolatcan-gyte_cse on 17.11.2016.
//
#include <osgWidget/Canvas>
#include <osg/Object>
#include <osg/Node>
#include <osg/Transform>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowMap>
#include <osgWidget/WindowManager>
#include <osgWidget/ViewerEventHandlers>
#include <iostream>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// TODO Keyboard handler for define position of target point
// TODO Integrate serial communication
// TODO Show numeric coordinates of ball (X,Y)

using namespace std;
using namespace osg;
using namespace osgViewer;
using namespace osgGA;
using namespace osgShadow;
using namespace osgDB;

// ------------------------------------ FUNCTIONS AND CALLBACKS -----------------------------
void setTextureSettings(Texture2D *shapeTexture,
                        Image *textureImage,
                        ref_ptr<ShapeDrawable> shape);
void createComponents();
void placeComponentsToScene();
void applyTexturesToComponents();
void applyShadows();
void placeCamera();
void setupWindowAndUI();
Matrix getRotator(float rotateVal, Vec3 rotateAxis);
Matrix getTranslator(Vec3f translateVal);
void rotatePropellerX(float rotateVal, Vec3 rotateAxis);
void rotatePropellerY(float rotateVal, Vec3 rotateAxis);
void liftPushRodX(Vec3 liftVal);
void liftPushRodY(Vec3 liftVal);
void * connectToQT(void *params);
//-----------------------------------------------------------------------------------------

const int ReceivesShadowTraversalMask = 0x1;
const int CastsShadowTraversalMask = 0x2;

// ---------------------------- COMPONENTS ------------------------------------
ref_ptr<ShadowedScene> shadowedScene = new ShadowedScene;
ref_ptr<ShapeDrawable> table;
ref_ptr<ShapeDrawable> bottomTable;
ref_ptr<ShapeDrawable> ball;
ref_ptr<ShapeDrawable> stick;
ref_ptr<ShapeDrawable> backingY;
ref_ptr<ShapeDrawable> backingX;
ref_ptr<ShapeDrawable> motorY;
ref_ptr<ShapeDrawable> motorX;
ref_ptr<ShapeDrawable> motorHeadY;
ref_ptr<ShapeDrawable> motorHeadX;
ref_ptr<ShapeDrawable> motorPropellerY;
ref_ptr<ShapeDrawable> motorPropellerX;
ref_ptr<ShapeDrawable> motorPropellerIgnoredY;
ref_ptr<ShapeDrawable> motorPropellerIgnoredX;
ref_ptr<ShapeDrawable> pushrodY;
ref_ptr<ShapeDrawable> pushrodX;

Group* scene;
Viewer viewer;
MatrixTransform *tableRotationMatrix = new MatrixTransform;
MatrixTransform *ballRotationMatrix = new MatrixTransform;
MatrixTransform *stickRotationMatrix = new MatrixTransform;
MatrixTransform *bottomTableRotationMatrix = new MatrixTransform;
MatrixTransform *pushrodRotationMatrixX = new MatrixTransform;
MatrixTransform *pushrodRotationMatrixY = new MatrixTransform;
MatrixTransform *pushrodTranslationMatrixX = new MatrixTransform;
MatrixTransform *pushrodTranslationMatrixY = new MatrixTransform;
MatrixTransform *propellerRotationMatrixX = new MatrixTransform;
MatrixTransform *propellerRotationMatrixY = new MatrixTransform;
MatrixTransform *tableRotationMatrixZero = new MatrixTransform;
PositionAttitudeTransform *ballPosition = new PositionAttitudeTransform();

Vec3 propellerXOrigin(-11.0f,-6.5f,-1.0f);
Vec3 propellerYOrigin(-3.0f,-6.5f,7.0f);
Vec3 ballOrigin(0.0f,0.0f,0.0f);
float BALL_ROTATION_ANGLE = 20.0f;
float TABLE_ROTATION_ANGLE_X;
float TABLE_ROTATION_ANGLE_Y;
float PROPELLER_ROTATION_ANGLE = 1.0f;
float PUSHROD_MOVING_POSITION = 0.1; // Move the pushrod by this amount
float BALL_POSITION_CHANGER = 0.35; // Change the ball position by this amount
//-----------------------------------------------------------------------------


class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:

    KeyboardEventHandler()
    {
    }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
    {
        switch (ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYDOWN):
            {
                switch (ea.getKey())
                {
                    case 'a':
                    {
                        // New ball translation technique
                        Vec3d oldPosition = ballPosition->getPosition(); // get old position of ball
                        // TODO This will be modified with touchscreen data
                        ballPosition->setPosition(Vec3d(oldPosition.x()+0.35,oldPosition.y(),oldPosition.z()));

                        // Rotating a table
                        tableRotationMatrix->setMatrix(getRotator(TABLE_ROTATION_ANGLE_X,Vec3(0,0,-1))*tableRotationMatrix->getMatrix());
                        // ---------------

                        // Rolling a ball
                        ballRotationMatrix->setMatrix(getRotator(BALL_ROTATION_ANGLE,Vec3(0,1,0))*ballRotationMatrix->getMatrix());

                        // Rotate servo propeller left
                        rotatePropellerX(PROPELLER_ROTATION_ANGLE,Vec3(1,0,0));
                        // -----------------------------

                        // Lift up pushrod left
                        liftPushRodX(Vec3f(0,0.1,0));
                        // -----------------------------
                    }
                        break;

                    case 's':
                    {
                        // New ball translation technique
                        Vec3d oldPosition = ballPosition->getPosition(); // get old position of ball
                        // TODO This will be modified with touchscreen data
                        ballPosition->setPosition(Vec3d(oldPosition.x(),oldPosition.y(),oldPosition.z()-0.35));

                        // Rotating a table
                        tableRotationMatrix->setMatrix(getRotator(TABLE_ROTATION_ANGLE_X,Vec3(-1,0,0))*tableRotationMatrix->getMatrix());

                        // Rolling a ball
                        ballRotationMatrix->setMatrix(getRotator(BALL_ROTATION_ANGLE,Vec3(0,1,0))*ballRotationMatrix->getMatrix());

                        // Rotate servo propeller right
                        rotatePropellerY(PROPELLER_ROTATION_ANGLE,Vec3(1,0,0));
                        // -----------------------------

                        // Lift up pushrod right
                        liftPushRodY(Vec3f(0,0.1,0));
                        // -----------------------------
                    }
                        break;

                    case 'd':
                    {
                        // New ball translation technique
                        Vec3d oldPosition = ballPosition->getPosition(); // get old position of ball
                        // TODO This will be modified with touchscreen data
                        ballPosition->setPosition(Vec3d(oldPosition.x()-0.35,oldPosition.y(),oldPosition.z()));

                        // Rotating a table
                        tableRotationMatrix->setMatrix(getRotator(TABLE_ROTATION_ANGLE_X,Vec3(0,0,1))*tableRotationMatrix->getMatrix());

                        // Rolling a ball
                        ballRotationMatrix->setMatrix(getRotator(20,Vec3(0,1,0)*ballRotationMatrix->getMatrix()));

                        // Rotate servo propeller left
                        rotatePropellerX(PROPELLER_ROTATION_ANGLE,Vec3(-1,0,0));
                        // -----------------------------

                        // Lift down pushrod right
                        liftPushRodX(Vec3f(0,-0.1f,0));
                        // -----------------------------
                    }
                        break;

                    case 'w':
                    {
                        // New ball translation technique
                        Vec3d oldPosition = ballPosition->getPosition(); // get old position of ball
                        // TODO This will be modified with touchscreen data
                        ballPosition->setPosition(Vec3d(oldPosition.x(),oldPosition.y(),oldPosition.z()+0.35));

                        // Rotating a table
                        tableRotationMatrix->setMatrix(getRotator(TABLE_ROTATION_ANGLE_X,Vec3(1,0,0))*tableRotationMatrix->getMatrix());

                        // Rolling a ball
                        ballRotationMatrix->setMatrix(getRotator(BALL_ROTATION_ANGLE,Vec3d(0,1,0))*ballRotationMatrix->getMatrix());

                        // Rotate servo propeller
                        rotatePropellerY(PROPELLER_ROTATION_ANGLE,Vec3(-1,0,0));
                        // -----------------------------

                        // Lift up pushrod
                        liftPushRodY(Vec3f(0,-0.1f,0));
                        // -----------------------------
                    }
                        break;

                    case 'q':
                    {
                        // New ball translation technique
                        Vec3d oldPosition = ballPosition->getPosition(); // get old position of ball
                        // TODO This will be modified with touchscreen data
                        ballPosition->setPosition(Vec3d(oldPosition.x()+0.35,oldPosition.y(),oldPosition.z()+0.35));

                        // Rotating a table
                        tableRotationMatrix->setMatrix(getRotator(TABLE_ROTATION_ANGLE_X,Vec3(1,0,-1))*tableRotationMatrix->getMatrix());

                        // Rolling a ball
                        ballRotationMatrix->setMatrix(getRotator(BALL_ROTATION_ANGLE,Vec3(0,1,0))*ballRotationMatrix->getMatrix());

                        // Rotate servo propeller left and right
                        rotatePropellerX(PROPELLER_ROTATION_ANGLE,Vec3(1,0,0));
                        rotatePropellerY(PROPELLER_ROTATION_ANGLE,Vec3(-1,0,0));
                        // -----------------------------

                        // Lift down push rod right, up left
                        liftPushRodX(Vec3f(0,0.1f,0));
                        liftPushRodY(Vec3f(0,-0.1f,0));
                        // -----------------------------
                    }
                        break;

                    case 'e':
                    {
                        // New ball translation technique
                        Vec3d oldPosition = ballPosition->getPosition(); // get old position of ball
                        // TODO This wil be modified with touchscreen data
                        ballPosition->setPosition(Vec3d(oldPosition.x()-0.35,oldPosition.y(),oldPosition.z()+0.35));

                        // Rotating a table
                        tableRotationMatrix->setMatrix(getRotator(TABLE_ROTATION_ANGLE_X,Vec3(1,0,1))*tableRotationMatrix->getMatrix());

                        // Rolling a ball
                        ballRotationMatrix->setMatrix(getRotator(BALL_ROTATION_ANGLE,Vec3(0,1,0))*ballRotationMatrix->getMatrix());

                        // Rotate servo propeller left and right
                        rotatePropellerX(PROPELLER_ROTATION_ANGLE,Vec3(-1,0,0));
                        rotatePropellerY(PROPELLER_ROTATION_ANGLE,Vec3(-1,0,0));
                        // -----------------------------

                        // Lift down pushrod right
                        liftPushRodX(Vec3f(0,-0.1f,0));
                        liftPushRodY(Vec3f(0,-0.1f,0));
                        // -----------------------------
                    }
                        break;


                    case 'z':
                    {
                        // New ball translation technique
                        Vec3d oldPosition = ballPosition->getPosition(); // get old position of ball
                        // TODO This wil be modified with touchscreen data
                        ballPosition->setPosition(Vec3d(oldPosition.x()+0.35,oldPosition.y(),oldPosition.z()-0.35));


                        // Rotating a table
                        tableRotationMatrix->setMatrix(getRotator(TABLE_ROTATION_ANGLE_X,Vec3(-1,0,-1))*tableRotationMatrix->getMatrix());

                        // Rolling a ball
                        ballRotationMatrix->setMatrix(getRotator(BALL_ROTATION_ANGLE,Vec3(0,1,0))*ballRotationMatrix->getMatrix());

                        // Rotate servo propeller left and right
                        rotatePropellerX(PROPELLER_ROTATION_ANGLE,Vec3(1,0,0));
                        rotatePropellerY(PROPELLER_ROTATION_ANGLE,Vec3(1,0,0));
                        // -----------------------------

                        // Lift down pushrod right
                        liftPushRodX(Vec3f(0,0.1f,0));
                        liftPushRodY(Vec3f(0,0.1f,0));
                        // -----------------------------
                    }
                        break;

                    case 'c':
                    {
                        // New ball translation technique
                        Vec3d oldPosition = ballPosition->getPosition(); // get old position of ball
                        // TODO This wil be modified with touchscreen data
                        ballPosition->setPosition(Vec3d(oldPosition.x()-0.35,oldPosition.y(),oldPosition.z()-0.35));

                        // Rotating a table
                        tableRotationMatrix->setMatrix(getRotator(TABLE_ROTATION_ANGLE_X,Vec3(-1,0,1))*tableRotationMatrix->getMatrix());

                        // Rolling a ball
                        ballRotationMatrix->setMatrix(getRotator(BALL_ROTATION_ANGLE,Vec3(0,1,0))*ballRotationMatrix->getMatrix());

                        // Rotate servo propeller left and right
                        rotatePropellerX(PROPELLER_ROTATION_ANGLE,Vec3(-1,0,0));
                        rotatePropellerY(PROPELLER_ROTATION_ANGLE,Vec3(1,0,0));
                        // -----------------------------

                        // Lift down pushrod right
                        liftPushRodX(Vec3f(0,-0.1f,0));
                        liftPushRodY(Vec3f(0,0.1f,0));
                        // -----------------------------
                    }
                        break;
                }
            }
            default:
                break;

        }
        //return false to allow mouse manipulation
        return false;
    }
};


int main(int argc, char** argv) {
    pthread_attr_t attrs;
    pthread_t tid;

    // Creating communication with QT
    pthread_attr_init(&attrs);
    pthread_attr_setdetachstate(&attrs,PTHREAD_CREATE_DETACHED);

    pthread_create(&tid,&attrs,connectToQT,NULL);

    createComponents();
    applyShadows();
    applyTexturesToComponents();
    placeComponentsToScene();
    placeCamera();
    setupWindowAndUI();

    return viewer.run();
}


/* coorX - X coordinate - oldPosX
 * coorY - Y coordinate - oldPosZ
 * dx    - Direction X moving amout
 * dy    - Direction Y moving amout
 * tableDegree - rotate the table by this degree
 * TRV - Table Rotating Vector - rotate the table by this vector
 * key - checks in which direction the propellers and pushrods move
 */
void doRotations(int coorX, int coorY, float dX, float dY, int tableDegree,
                 Vec3f TRV, char key){

    // Get the old position of the ball
    Vec3d oldPosition = ballPosition->getPosition();

    Matrix tableRotation, ballRotation;
    if (TABLE_ROTATION_ANGLE_X > 0)
         tableRotation.makeRotate(TABLE_ROTATION_ANGLE_X*PI/180, Vec3d(-1,0,0));
    if (TABLE_ROTATION_ANGLE_Y > 0)
        tableRotation.makeRotate(TABLE_ROTATION_ANGLE_Y*PI/180, Vec3d(0,0,-1));
    if (TABLE_ROTATION_ANGLE_X < 0)
        tableRotation.makeRotate(TABLE_ROTATION_ANGLE_X*PI/180, Vec3d(1,0,0));
    if (TABLE_ROTATION_ANGLE_Y < 0)
        tableRotation.makeRotate(TABLE_ROTATION_ANGLE_Y*PI/180, Vec3d(0,0,1));

    ballRotation.makeRotate(BALL_ROTATION_ANGLE*PI/180, Vec3d(0,1,0));


    for (int i = 0; i < tableDegree; ++i){
        // Rotate the table
        tableRotationMatrix->setMatrix(tableRotation*tableRotationMatrixZero->getMatrix());

        // Rotate the ball
        ballPosition->setPosition(Vec3d(coorX+dX,oldPosition.y(),coorY+dY));
        ballRotationMatrix->setMatrix(ballRotation*ballRotationMatrix->getMatrix());

        // Rotate the propellers and pushrods
        switch(key){
            case 'A':
                rotatePropellerX(PROPELLER_ROTATION_ANGLE,Vec3(1,0,0));
//                liftPushRodX(Vec3f(0,PUSHROD_MOVING_POSITION,0));
                break;
            case 'D':
                rotatePropellerX(PROPELLER_ROTATION_ANGLE,Vec3(-1,0,0));
//                liftPushRodX(Vec3f(0,-PUSHROD_MOVING_POSITION,0));
                break;
            case 'W':
                rotatePropellerY(PROPELLER_ROTATION_ANGLE,Vec3(-1,0,0));
//                liftPushRodY(Vec3f(0,-PUSHROD_MOVING_POSITION,0));
                break;
            case 'S':
                rotatePropellerY(PROPELLER_ROTATION_ANGLE,Vec3(1,0,0));
//                liftPushRodY(Vec3f(0,PUSHROD_MOVING_POSITION,0));
                break;
            case 'Q':
                rotatePropellerX(PROPELLER_ROTATION_ANGLE,Vec3(1,0,0));
                rotatePropellerY(PROPELLER_ROTATION_ANGLE,Vec3(-1,0,0));
//                liftPushRodX(Vec3f(0,PUSHROD_MOVING_POSITION,0));
//                liftPushRodY(Vec3f(0,-PUSHROD_MOVING_POSITION,0));
                break;
            case 'E':
                rotatePropellerX(PROPELLER_ROTATION_ANGLE,Vec3(-1,0,0));
                rotatePropellerY(PROPELLER_ROTATION_ANGLE,Vec3(-1,0,0));
//                liftPushRodX(Vec3f(0,-PUSHROD_MOVING_POSITION,0));
//                liftPushRodY(Vec3f(0,-PUSHROD_MOVING_POSITION,0));
                break;
            case 'Z':
                rotatePropellerX(PROPELLER_ROTATION_ANGLE,Vec3(1,0,0));
                rotatePropellerY(PROPELLER_ROTATION_ANGLE,Vec3(1,0,0));
//                liftPushRodX(Vec3f(0,PUSHROD_MOVING_POSITION,0));
//                liftPushRodY(Vec3f(0,PUSHROD_MOVING_POSITION,0));
                break;
            case 'C':
                rotatePropellerX(PROPELLER_ROTATION_ANGLE, Vec3(-1,0,0));
                rotatePropellerY(PROPELLER_ROTATION_ANGLE, Vec3(1,0,0));
//                liftPushRodX(Vec3f(0,-PUSHROD_MOVING_POSITION,0));
//                liftPushRodY(Vec3f(0,PUSHROD_MOVING_POSITION,0));
                break;
        }
    }
}

int oldAngleLeft = 90; // Previous angle of the left motor
int oldAngleRight = 100; // Previous angle of the right motor

/* coorX - X coordinate - oldPosX
 * coorY - Y coordinate - oldPosZ
 * angleRight - the angle of the right motor
 * angleLeft - the angle of the left motor
 */
void moveTheSystem(int coorX, int coorY, int angleRight, int angleLeft){
    float dX, dY;
    int tableDegree = (angleRight + angleLeft)/2;

    TABLE_ROTATION_ANGLE_X = (angleLeft - 90) / 5;
    TABLE_ROTATION_ANGLE_Y = (angleRight - 90) / 5;

    // Table rotation technique
    Matrix rotation;

    if(angleLeft < oldAngleLeft){
        dX = -BALL_POSITION_CHANGER;
        // Button D
        if(angleRight == oldAngleRight){
            dY = 0;
            doRotations(coorX, coorY, dX, dY, tableDegree, Vec3f(0, 0, 1), 'D');
        }
            // Button C
        else if(angleRight > oldAngleRight){
            dY = -BALL_POSITION_CHANGER;
            doRotations(coorX, coorY, dX, dY, tableDegree, Vec3f(-1, 0, 1), 'C');
        }
            // Button E
        else // angleRight < oldAngleRight
        {
            dY = BALL_POSITION_CHANGER;
            doRotations(coorX, coorY, dX, dY, tableDegree, Vec3f(1, 0, 1), 'E');
        }
    }
    else if(angleLeft > oldAngleLeft) {
        dX = BALL_POSITION_CHANGER;
        // Button A
        if (angleRight == oldAngleRight) {
            dY = 0;
            doRotations(coorX, coorY, dX, dY, tableDegree, Vec3f(0, 0, -1), 'A');
        }
            // Button Z
        else if (angleRight > oldAngleRight) {
            dY = -BALL_POSITION_CHANGER;
            doRotations(coorX, coorY, dX, dY, tableDegree, Vec3f(-1, 0, -1), 'Z');
        }
            // Button Q
        else // angleRight < oldAngleRight
        {
            dY = BALL_POSITION_CHANGER;
            doRotations(coorX, coorY, dX, dY, tableDegree, Vec3f(1, 0, -1), 'Q');
        }
    }
    else // angleLeft == oldAngleRight
    {
        dX = 0;
        // NO ACTION
        if (angleRight == oldAngleRight) {
            dY = 0;
        }
            // Button S
        else if (angleRight > oldAngleRight) {
            dY = -BALL_POSITION_CHANGER;
            doRotations(coorX, coorY, dX, dY, tableDegree, Vec3f(-1, 0, 0), 'S');
        }
            // Button W
        else // angleRight < oldAngleRight
        {
            dY = BALL_POSITION_CHANGER;
            doRotations(coorX, coorY, dX, dY, tableDegree, Vec3f(1, 0, 0), 'W');
        }
    }

    oldAngleLeft = angleLeft;
    oldAngleRight = angleRight;
}

void setupWindowAndUI() {
    ref_ptr<osgWidget::WindowManager> wm = new osgWidget::WindowManager(&viewer,800.0f,640.0f,0xf0000000);
    Camera* widgetCamera = wm->createParentOrthoCamera();

    wm->resizeAllWindows();

    scene->addChild(widgetCamera);

    viewer.setSceneData(scene);
    viewer.setUpViewInWindow(20,20,800,640);
    viewer.addEventHandler(new KeyboardEventHandler());
    viewer.addEventHandler(new osgWidget::MouseHandler(wm.get()));
    viewer.addEventHandler(new osgWidget::KeyboardHandler(wm.get()));
    viewer.addEventHandler(new osgWidget::ResizeHandler(wm.get(),widgetCamera));
    viewer.addEventHandler(new osgWidget::CameraSwitchHandler(wm.get(),widgetCamera));
}

void createComponents() {
    table = new ShapeDrawable;
    ball = new ShapeDrawable;
    stick = new ShapeDrawable;
    bottomTable = new ShapeDrawable;

    //mehmed and yunus was here
    backingY = new ShapeDrawable;
    backingX = new ShapeDrawable;
    motorY = new ShapeDrawable;
    motorX = new ShapeDrawable;
    motorHeadY = new ShapeDrawable;
    motorHeadX = new ShapeDrawable;
    motorPropellerY = new ShapeDrawable;
    motorPropellerX = new ShapeDrawable;
    motorPropellerIgnoredY = new ShapeDrawable;
    motorPropellerIgnoredX = new ShapeDrawable;
    pushrodX = new ShapeDrawable;
    pushrodY = new ShapeDrawable;

    scene = new Group();

    table->setShape(new Box(Vec3(0.0,0.0,0.0),30.0f,0.5f,20.0f));
    ball->setShape(new Sphere(Vec3(0.0,1.20f,0.0f),1.0f));
    stick->setShape(new Cylinder(Vec3(0.0f,0.0f,8.0f),1.2f,15.0f));
    bottomTable->setShape(new Box(Vec3(0.0,-15.0f,0.0f),40.0f,1.0f,30.0f));

    //mehmed and yunus was here
    backingY->setShape(new Box(Vec3(0.0f,-12.0f,8.0f),3.75f,6.0f,2.5f));
    backingX->setShape(new Box(Vec3(-8.0f,-12.0f,0.0f),3.75f,6.0f,2.5f));
    motorY->setShape(new Box(Vec3(-1.5f,-7.5f,8.0f),2.0f,3.0f,2.5f));
    motorX->setShape(new Box(Vec3(-9.5f,-7.5f,0.0f),2.0f,3.0f,2.5f));
    motorHeadY->setShape(new Box(Vec3(-2.5f,-6.5f,8.0f),0.9f,0.4f,0.4f));
    motorHeadX->setShape(new Box(Vec3(-10.5f,-6.5f,0.0f),0.9f,0.4f,0.4f));
    motorPropellerY->setShape(new Box(Vec3(-3.0f,-6.5f,7.0f),0.2f,0.8f,5.0f));
    motorPropellerX->setShape(new Box(Vec3(-11.0f,-6.5f,-1.0f),0.2f,0.8f,5.0f));
    motorPropellerIgnoredY->setShape(new Box(Vec3(-3.0f,-6.5f,8.0f),0.2f,3.0f,0.8f));
    motorPropellerIgnoredX->setShape(new Box(Vec3(-11.0f,-6.5f,0.0f),0.2f,3.0f,0.8f));
    pushrodY->setShape(new Cylinder(Vec3(-3.0f,5.0f,3.8f),0.2f,8.0f));
    pushrodX->setShape(new Cylinder(Vec3(-11.0f,-3.0f,3.8f),0.2f,8.0f));
}

void applyShadows() {
    // Shadowing effects on ball
    shadowedScene->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
    shadowedScene->setCastsShadowTraversalMask(CastsShadowTraversalMask);
    ref_ptr<ShadowMap> sm = new ShadowMap;
    shadowedScene->setShadowTechnique(sm.get());

    scene->addChild(shadowedScene);

    table->setNodeMask(ReceivesShadowTraversalMask);
    ball->setNodeMask(CastsShadowTraversalMask);
}

void applyTexturesToComponents() {
    Image *ballImage = readImageFile("metal_texture_3.bmp");
    Image *tableImage = readImageFile("screen_texture.bmp");
    Image *stickImage = readImageFile("stick_texture.bmp");
    Image *bottomTableImage = readImageFile("wood_texture.bmp");

    //mehmed and yunus and alper was here
    Image *backingImage = readImageFile("backing.bmp");
    Image *motorImage = readImageFile("motor.bmp");
    Image *motorHeadImage = readImageFile("motorHead.bmp");
    Image *motorPropellerImage = readImageFile("motorPropeller.bmp");

    Texture2D *ballTexture = new Texture2D;
    Texture2D *tableTexture = new Texture2D;
    Texture2D *stickTexture = new Texture2D;
    Texture2D *bottomTableTexture = new Texture2D;

    //mehmed and yunus and alper was here
    Texture2D *backingTexture = new Texture2D;
    Texture2D *motorTexture = new Texture2D;
    Texture2D *motorHeadTexture = new Texture2D;
    Texture2D *motorPropellerTexture = new Texture2D;
    //
    if (ballImage == NULL || tableImage == NULL || stickImage == NULL || bottomTableImage == NULL ||
        backingImage == NULL || motorImage == NULL || motorHeadImage== NULL ||
        motorPropellerImage == NULL)
        cout << "Textures can't be loaded!" << endl;

    setTextureSettings(ballTexture,ballImage,ball);
    setTextureSettings(tableTexture,tableImage,table);
    setTextureSettings(stickTexture,stickImage,stick);
    setTextureSettings(bottomTableTexture,bottomTableImage,bottomTable);

    //mehmed and yunus was here
    setTextureSettings(backingTexture,backingImage,backingY);
    setTextureSettings(backingTexture,backingImage,backingX);
    setTextureSettings(motorTexture,motorImage,motorY);
    setTextureSettings(motorTexture,motorImage,motorX);
    setTextureSettings(motorHeadTexture,motorHeadImage,motorHeadY);
    setTextureSettings(motorHeadTexture,motorHeadImage,motorHeadX);
    setTextureSettings(motorPropellerTexture,motorPropellerImage,motorPropellerY);
    setTextureSettings(motorPropellerTexture,motorPropellerImage,motorPropellerX);
    setTextureSettings(motorPropellerTexture,motorPropellerImage,motorPropellerIgnoredY);
    setTextureSettings(motorPropellerTexture,motorPropellerImage,motorPropellerIgnoredX);
    setTextureSettings(motorTexture,motorImage,pushrodY);
    setTextureSettings(motorTexture,motorImage,pushrodX);
    //

}

void placeComponentsToScene() {
    // Bottom table of mechanic system
    bottomTableRotationMatrix->addChild(bottomTable.get());
    Matrix rotateBottomTable;
    rotateBottomTable.makeRotate(180*PI/180,0,1,0);
    bottomTableRotationMatrix->setMatrix(rotateBottomTable*bottomTableRotationMatrix->getMatrix());
    scene->addChild(bottomTableRotationMatrix);
    // -------------------------------------------------

    // Stick of mechanic system
    stickRotationMatrix->addChild(stick.get());
    Matrix rotateStick;
    rotateStick.makeRotate(90*PI/180,1,0,0);
    stickRotationMatrix->setMatrix(rotateStick*stickRotationMatrix->getMatrix());
    scene->addChild(stickRotationMatrix);
    // -------------------------------------------------

    // Ball, table and target point components' placement in there. Ball and table
    // collision detection applied.
    ballRotationMatrix->addChild(ball.get());
    tableRotationMatrix->addChild(table.get());
    tableRotationMatrix->addChild(ballPosition);
    ballPosition->setPosition(Vec3(0.0,0.0,0.0));
    ballPosition->addChild(ballRotationMatrix);
    shadowedScene->addChild(tableRotationMatrix);
    //--------------------------------------------------

//     Pushrods (Çubuklar)
    pushrodRotationMatrixY->addChild(pushrodY.get());
    pushrodRotationMatrixX->addChild(pushrodX.get());
    Matrix rotatePushrod;
    rotatePushrod.makeRotate(90*PI/180,1,0,0);
    pushrodRotationMatrixX->setMatrix(rotatePushrod*pushrodRotationMatrixX->getMatrix());
    pushrodRotationMatrixY->setMatrix(rotatePushrod*pushrodRotationMatrixY->getMatrix());

    pushrodTranslationMatrixX->addChild(pushrodRotationMatrixX);
    pushrodTranslationMatrixY->addChild(pushrodRotationMatrixY);

    scene->addChild(pushrodTranslationMatrixX);
    scene->addChild(pushrodTranslationMatrixY);
//     ----------------------------------------------------------------------------

    propellerRotationMatrixX->addChild(motorPropellerX);
    propellerRotationMatrixY->addChild(motorPropellerY);

    scene->addChild(propellerRotationMatrixX);
    scene->addChild(propellerRotationMatrixY);

//     mehmed and yunus was here.
    scene->addChild(backingY);
    scene->addChild(backingX);
    scene->addChild(motorY);
    scene->addChild(motorX);
    scene->addChild(motorHeadY);
    scene->addChild(motorHeadX);
    scene->addChild(motorPropellerIgnoredY);
    scene->addChild(motorPropellerIgnoredX);
}

void placeCamera() {
    // --------------------- 3D Camera position settings ------------------------
    Vec3d eye, center, up;

    viewer.addEventHandler(new KeyboardEventHandler());
    viewer.setCameraManipulator(new TrackballManipulator);
    viewer.getCameraManipulator()->getHomePosition(eye,center,up);
    viewer.getCameraManipulator()->setHomePosition(-Vec3d(eye.x(),eye.y()-70,eye.z()-60),
                                                   Vec3d(center.x(),center.y(),center.z()),
                                                   -Vec3d(up.x(),up.y(),up.z()));
    /*--------------------------------------------------------------------------*/
}

// Applies textures to corresponding piece of mechanic system
void setTextureSettings(Texture2D *shapeTexture, Image *textureImage, ref_ptr<ShapeDrawable> shape) {
    Material *material = new Material();
    material->setEmission(Material::FRONT, Vec4(0.8,0.8,0.8,1.0));

    shapeTexture->setDataVariance(Object::DYNAMIC);
    shapeTexture->setFilter(Texture::MIN_FILTER,Texture::LINEAR_MIPMAP_LINEAR);
    shapeTexture->setFilter(Texture::MAG_FILTER,Texture::LINEAR);
    shapeTexture->setWrap(Texture::WRAP_S,Texture::CLAMP);
    shapeTexture->setWrap(Texture::WRAP_T,Texture::CLAMP);
    shapeTexture->setImage(textureImage);

    StateSet *stateSet = shape->getOrCreateStateSet();
    stateSet->ref();
    stateSet->setAttribute(material);
    stateSet->setTextureAttributeAndModes(0,shapeTexture,StateAttribute::ON);
}

Matrix getTranslator(Vec3f translateVal) {
    Matrix translator;
    translator.makeTranslate(translateVal);

    return translator;
}

Matrix getRotator(float rotateVal, Vec3 rotateAxis) {
    Matrix rotator;

    rotator.makeRotate(rotateVal*PI/180,rotateAxis.x(),rotateAxis.y(),rotateAxis.z());

    return rotator;
}

void rotatePropellerX(float rotateVal, Vec3 rotateAxis) {
    propellerRotationMatrixX->setMatrix(getTranslator(propellerXOrigin)*propellerRotationMatrixX->getMatrix());
    propellerRotationMatrixX->setMatrix(getRotator(rotateVal,rotateAxis)*propellerRotationMatrixX->getMatrix());
    propellerRotationMatrixX->setMatrix(getTranslator(-propellerXOrigin)*propellerRotationMatrixX->getMatrix());
}

void rotatePropellerY(float rotateVal, Vec3 rotateAxis) {
    propellerRotationMatrixY->setMatrix(getTranslator(propellerYOrigin)*propellerRotationMatrixY->getMatrix());
    propellerRotationMatrixY->setMatrix(getRotator(rotateVal,rotateAxis)*propellerRotationMatrixY->getMatrix());
    propellerRotationMatrixY->setMatrix(getTranslator(-propellerYOrigin)*propellerRotationMatrixY->getMatrix());
}

void liftPushRodX(Vec3 liftVal) {
    pushrodTranslationMatrixX->setMatrix(getTranslator(liftVal)*pushrodTranslationMatrixX->getMatrix());
}

void liftPushRodY(Vec3 liftVal) {
    pushrodTranslationMatrixY->setMatrix(getTranslator(liftVal)*pushrodTranslationMatrixY->getMatrix());
}

void * connectToQT(void *) {
    int s;
    struct sockaddr_in serverAddr;


    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cout << "Could not create socket " << endl;
    }

    cout << "Socket created" << endl;

    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1234);

    // Connect to server
    if (connect(s, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        cout << "Connect error" << endl;
    }

    cout << "Connected" << endl;

    for (;;) {
        char receivedData[64];
        int dataSize = recv(s, receivedData, 64, 0);
        if (dataSize > 0)
            receivedData[dataSize] = '\0';

        cout << "Received data: " << receivedData << endl;

        int x = atoi(strtok(receivedData, ","));
        int y = atoi(strtok(NULL, ","));
        int angleX = atoi(strtok(NULL, ","));
        int angleY = atoi(strtok(NULL, ","));

        cout << "X: " << x << endl;
        cout << "Y: " << y << endl << endl;
        cout << "angleX: " << angleX << endl;
        cout << "angleY: " << angleY << endl;

        ballPosition->setPosition(Vec3((x-495)/37,ballPosition->getPosition().y(),(y-495)/40));
        memset(receivedData, '\0', 64);
        moveTheSystem(x,y,angleX,angleY);
//        sleep(1);
//                cout << "Data: " << receivedData << " received from client" << endl;
//        cout << "Data Size: " << dataSize << endl;
    }
}