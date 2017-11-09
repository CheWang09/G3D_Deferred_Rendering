#include <G3D/G3DAll.h>
#include <GLG3D/GLG3D.h>
#define SCREEN_HEIGHT 768
#define SCREEN_WIDTH 1024
class App : public GApp {
private:
shared_ptr<ArticulatedModel> church,bunny,quad;
shared_ptr<G3D::Texture> tex_shadow,tex_shadow_depth,tex_pos,tex_norm,tex_depth,tex_vis;
shared_ptr<Framebuffer> g_framebuffer,g_framebuffer_def;
G3D::Vector3 light_pos;
float light_spot_angle;
std::string shader_base_path;
shared_ptr<ThirdPersonManipulator> manipulator;
G3D::Matrix4 rotate;
float rotate_angle;
public:
App(const GApp::Settings& settings) ;
virtual void onInit();
virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D);
void createScene();
void onUserInput(class UserInput* userInput);
void processTexture(RenderDevice* rd,shared_ptr<Framebuffer> framebuf,shared_ptr<ArticulatedModel>
quad,std::string shader_base_path);
};
App::App(const GApp::Settings& settings):GApp(settings){
rotate_angle = 0;
}
void App::createScene()
{
ArticulatedModel::Specification spec;
//
spec.filename = System::findDataFile("C:/G3D9/data/3ds/fantasy/church/kosciol.3ds"); //
spec.scale = 0.050f;
spec.stripMaterials = false;
church = ArticulatedModel::create(spec,"church");
//
spec.filename = System::findDataFile("bunny.ifs");//
spec.scale = 0.1f;
spec.stripMaterials = false;
bunny = ArticulatedModel::create(spec,"cube");
spec.filename = "C:/G3D9/data/quad.obj";
spec.scale = 1.0f;
spec.stripMaterials = false;
quad = ArticulatedModel::create(spec,"quad");
// settings for shadow map
g_framebuffer = Framebuffer::create("gbuffer fbo");
tex_shadow =
Texture::createEmpty("depth_buffer",768,768,ImageFormat::RGB32F(),Texture::DIM_2D_NPOT,Texture::Settings::buffer());
tex_shadow_depth =
Texture::createEmpty("depth_test",768,768,ImageFormat::DEPTH32F(),Texture::DIM_2D_NPOT,Texture::Settings::buffer());
g_framebuffer->set(Framebuffer::COLOR0, tex_shadow);
g_framebuffer->set(Framebuffer::DEPTH, tex_shadow_depth);
// settings for deferred shading
g_framebuffer_def = Framebuffer::create("gbuffer fbo");
tex_pos =
Texture::createEmpty("frag_pos",SCREEN_WIDTH,SCREEN_HEIGHT,ImageFormat::RGB32F(),Texture::DIM_2D_NPOT,Texture::Settings::buffer());
tex_norm =
Texture::createEmpty("frag_pos",SCREEN_WIDTH,SCREEN_HEIGHT,ImageFormat::RGB32F(),Texture::DIM_2D_NPOT,Texture::Settings::buffer());
tex_depth =
Texture::createEmpty("depth",SCREEN_WIDTH,SCREEN_HEIGHT,ImageFormat::DEPTH32F(),Texture::DIM_2D_NPOT,Texture::Settings::buffer());
tex_vis =
Texture::createEmpty("frag_pos",SCREEN_WIDTH,SCREEN_HEIGHT,ImageFormat::RGB32F(),Texture::DIM_2D_NPOT,Texture::Settings::buffer());
g_framebuffer_def->set(Framebuffer::COLOR0,tex_pos);
g_framebuffer_def->set(Framebuffer::COLOR1,tex_norm);
g_framebuffer_def->set(Framebuffer::COLOR2,tex_vis);
g_framebuffer_def->set(Framebuffer::DEPTH,tex_depth);
}
void App::onInit() {
createDeveloperHUD();
window()->setCaption("church");
//
createScene();
light_pos = G3D::Vector3(0.00001,2,0.0);
light_spot_angle = 6; // degree
//shader_base_path = "E:/G3D9.0/G3D9/samples/pixelShader/"; //
m_debugCamera->setPosition(Vector3(2.00001,2,0.0));
m_debugCamera->setFieldOfView(12 * units::degrees(), FOVDirection::VERTICAL);
m_debugCamera->lookAt(Point3::zero());
//
manipulator = ThirdPersonManipulator::create();
addWidget(manipulator);
// Turn off the default first-person camera controller and developer UI
m_debugController->setEnabled(false);
developerWindow->setVisible(false);
developerWindow->cameraControlWindow->setVisible(false);
showRenderingStats = true;
rotate = Matrix4::identity();
//
glShadeModel(GL_SMOOTH);
}
void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) {
Args args;
G3D::Vector3 translate;
// first rendering pass----------------------------------
rd->pushState(g_framebuffer);
G3D::Camera myCamera(*m_debugCamera);
myCamera.setPosition(light_pos);
myCamera.lookAt(Point3::zero());
myCamera.setFieldOfView(light_spot_angle*2 * units::degrees(), FOVDirection::VERTICAL);
rd->setProjectionAndCameraMatrix(myCamera.projection(), myCamera.frame());
G3D::Matrix4 M = rd->modelViewProjectionMatrix();
rd->setColorClearValue(G3D::Color4(10.0f,10.0f,10.0f,10.0f));
rd->clear(true,true,true);
rd->setDepthWrite(true);
rd->setDepthTest(RenderDevice:: DEPTH_LESS);
Array< shared_ptr<Surface> > mySurfaces;
// load church--------------------
glMatrixMode(GL_MODELVIEW);
glPushMatrix();
glScalef(1,1,1);
translate = G3D::Vector3(-0.0,-0.5,0.0);
rd->setColor(G3D::Color3(0.3f,0.2f,0.8f));
church->pose(mySurfaces, manipulator->frame());
for (int i = 0; i < mySurfaces.size(); ++i) {
shared_ptr<UniversalSurface> surface = dynamic_pointer_cast<UniversalSurface>(mySurfaces[i]);
surface->gpuGeom()->setShaderArgs(args);
args.setUniform("app_translate",translate);
args.setUniform("light_spot_angle",light_spot_angle);
args.setUniform("light_pos",light_pos);
args.setUniform("rotate",G3D::Matrix4::identity());
LAUNCH_SHADER("bunny_and_church.*", args);
}
glPopMatrix();
mySurfaces.clear();
// load bunny--------------------
glMatrixMode(GL_MODELVIEW);
glPushMatrix();
glScalef(1,1,1);
translate = G3D::Vector3(-0.1,-0.1,0.1);
rd->setColor(G3D::Color3(0.3f,0.2f,0.8f));
bunny->pose(mySurfaces, manipulator->frame());
for (int i = 0; i < mySurfaces.size(); ++i) {
shared_ptr<UniversalSurface> surface = dynamic_pointer_cast<UniversalSurface>(mySurfaces[i]);
surface->gpuGeom()->setShaderArgs(args);
args.setUniform("app_translate",translate);
args.setUniform("light_spot_angle",light_spot_angle);
args.setUniform("light_pos",light_pos);
args.setUniform("rotate",rotate);
LAUNCH_SHADER("bunny_and_church.*", args);
}
glPopMatrix();
mySurfaces.clear();
rd->popState();
// second rendering pass----------------------------------
rd->pushState(g_framebuffer_def);
rd->setProjectionAndCameraMatrix(m_debugCamera->projection(), m_debugCamera->frame());
rd->setColorClearValue(G3D::Color4(0.1f,0.2f,0.4f,0.1));
rd->clear(true,true,true);
rd->setDepthWrite(true);
rd->setDepthTest(RenderDevice:: DEPTH_LESS);
// load church--------------------
glMatrixMode(GL_MODELVIEW);
glPushMatrix();
//
glScalef(1,1,1);
translate = G3D::Vector3(-0.0,-0.5,0.0);
rd->setColor(G3D::Color3(0.6f,0.8f,0.2f));
church->pose(mySurfaces, manipulator->frame());
for (int i = 0; i < mySurfaces.size(); ++i) {
shared_ptr<UniversalSurface> surface = dynamic_pointer_cast<UniversalSurface>(mySurfaces[i]);
surface->gpuGeom()->setShaderArgs(args);
args.setUniform("app_translate",translate);
args.setUniform("light_spot_angle",light_spot_angle);
args.setUniform("light_pos",light_pos);
args.setUniform("sampler0",tex_shadow);
args.setUniform("M",M);
args.setUniform("rotate",G3D::Matrix4::identity());
LAUNCH_SHADER("bunny_church_2.*", args);
}
glPopMatrix();
mySurfaces.clear();
// load bunny--------------------
glMatrixMode(GL_MODELVIEW);
glPushMatrix();
//
glScalef(1,1,1);
translate = G3D::Vector3(-0.1,-0.1,0.1);
rd->setColor(G3D::Color3(0.3f,0.2f,0.8f));
bunny->pose(mySurfaces, manipulator->frame());
for (int i = 0; i < mySurfaces.size(); ++i) {
shared_ptr<UniversalSurface>surface=dynamic_pointer_cast<UniversalSurface>(mySurfaces[i]);
surface->gpuGeom()->setShaderArgs(args);
args.setUniform("app_translate",translate);
args.setUniform("light_spot_angle",light_spot_angle);
args.setUniform("light_pos",light_pos);
args.setUniform("sampler0",tex_shadow);
args.setUniform("M",M);
args.setUniform("rotate",rotate);
LAUNCH_SHADER("bunny_church_2.*", args);
}
glPopMatrix();
rd->popState();
//third rendering process - deferred shading
processTexture(rd,0,quad,shader_base_path);
}
Void App::processTexture(RenderDevice*rd,shared_ptr<Framebuffer> framebuf,shared_ptr<ArticulatedModel>
quad,std::string shader_base_path)
{
Array< shared_ptr<Surface> > mySurfaces;
Args args;
if(framebuf==0){
rd->pushState();
}
else{
rd->pushState(framebuf);
}
shared_ptr<G3D::Camera> screen_quad_cam = Camera::create("screen_quad");
screen_quad_cam->setPosition(Point3(0.0f,1.0f,0.0f));
screen_quad_cam->lookAt(Point3(0.0f,0.0f,0.0f));
int screen_width = rd->width();
int screen_height = rd->height();
Vector3 screen_scale(1.0f,1.0f,1.0f);
if(screen_width>screen_height)
{
screen_quad_cam->setFieldOfView(90*units::degrees(),FOVDirection::VERTICAL);
screen_scale.z= (double)screen_width / (double)screen_height;
}
else
{
screen_quad_cam->setFieldOfView(90*units::degrees(),FOVDirection::HORIZONTAL);
screen_scale.x= (double)screen_height / (double)screen_width;
}
rd->setProjectionAndCameraMatrix(screen_quad_cam->projection(), screen_quad_cam->frame());
rd->setColorClearValue(G3D::Color4(0.1f,0.2f,0.4f,0.1));
rd->clear(true,true,true);
// load quad--------------------
glMatrixMode(GL_MODELVIEW);
glPushMatrix();
quad->pose(mySurfaces, manipulator->frame());
for (int i = 0; i < mySurfaces.size(); ++i) {
shared_ptr<UniversalSurface>surface=dynamic_pointer_cast<UniversalSurface>(mySurfaces[i]);
surface->gpuGeom()->setShaderArgs(args);
args.setUniform("sampler_pos",tex_pos);
args.setUniform("sampler_norm",tex_norm);
args.setUniform("sampler_vis",tex_vis);
args.setUniform("screen_scale",screen_scale);
args.setUniform("height",screen_height);
args.setUniform("width",screen_width);
args.setUniform("light_pos",light_pos);
LAUNCH_SHADER("quad.*", args);
}
glPopMatrix();
mySurfaces.clear();
rd->popState();
}
void App::onUserInput(class UserInput* userInput)
{
if(userInput->keyPressed(G3D::GKey::F2))
{
rotate_angle = rotate_angle+5;
float angle = rotate_angle/180*3.1415926;
rotate=G3D::Matrix4(cos(angle),0,sin(angle),0,0,1,0,0,-sin(angle),0,cos(angle),0,
0,0,0,1);
}
}
G3D_START_AT_MAIN();
int main(int argc, const char* argv[]) {
initGLG3D();
GApp::Settings settings(argc, argv);
settings.window.caption = argv[0];
settings.window.width = SCREEN_WIDTH;
settings.window.height = SCREEN_HEIGHT;
settings.window.asynchronous = true;
settings.depthGuardBandThickness = Vector2int16(64, 64);
settings.colorGuardBandThickness = Vector2int16(16, 16);
settings.colorGuardBandThickness = Vector2int16(0, 0);
settings.depthGuardBandThickness = Vector2int16(0, 0);
settings.dataDir = "C:/G3D9/data";
return App(settings).run();
}
