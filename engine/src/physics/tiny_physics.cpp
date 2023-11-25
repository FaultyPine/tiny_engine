#include "tiny_physics.h"

#include "bullet/btBulletDynamicsCommon.h"
#include "tiny_alloc.h"
#include "tiny_engine.h"
#include "model.h"
#include "render/shapes.h"
#include "tiny_log.h"
#include "tiny_profiler.h"

btIDebugDraw* GetTinyPhysicsDebugDrawer(Arena* arena);

void InitializePhysics(Arena* arena)
{
    PROFILE_FUNCTION();
    PhysicsWorld* physWorld = (PhysicsWorld*)arena_alloc(arena, sizeof(PhysicsWorld));
    GetEngineCtx().physicsWorld = physWorld;


    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));

    physWorld->dynamicsWorld = dynamicsWorld;
    btIDebugDraw* debugDrawer = GetTinyPhysicsDebugDrawer(arena);
    physWorld->dynamicsWorld->setDebugDrawer(debugDrawer);
}

PhysicsWorld& GetPhysicsWorld() { return *GetEngineCtx().physicsWorld; }

void PhysicsTick()
{
    PROFILE_FUNCTION();
    PhysicsWorld& pw = GetPhysicsWorld();
    pw.dynamicsWorld->stepSimulation(GetDeltaTime());

}
void PhysicsDebugRender()
{
    PROFILE_FUNCTION();
    PhysicsWorld& pw = GetPhysicsWorld();
    btDiscreteDynamicsWorld* dynamicsWorld = pw.dynamicsWorld;

    if (dynamicsWorld && dynamicsWorld->getDebugDrawer())
    {
        dynamicsWorld->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
        dynamicsWorld->debugDrawWorld();
    }
}
void PhysicsAddModel(const Model& model, const Transform& tf)
{
    PROFILE_FUNCTION();
    PhysicsWorld& pw = GetPhysicsWorld();
    btCompoundShape* shape = new btCompoundShape();

    for (const Mesh& mesh : model.meshes)
    {
        BoundingBox meshbounds = mesh.cachedBoundingBox;
        glm::vec3 halfExtentsGlm = meshbounds.halfExtents() * tf.scale;
        glm::vec3 center = meshbounds.center() * tf.scale;
        btVector3 halfExtents = btVector3(halfExtentsGlm.x, halfExtentsGlm.y, halfExtentsGlm.z); 
        btBoxShape* boundingBoxCollisionShape = new btBoxShape(halfExtents);
        btVector3 meshPos = btVector3(center.x, center.y, center.z);
        btTransform transform = btTransform(btQuaternion::getIdentity(), meshPos);
        shape->addChildShape(transform, boundingBoxCollisionShape);
    }
    btTransform startTransform;
    startTransform.setIdentity();
    btScalar mass(0.f);
    // static rigidbody
    btVector3 localInertia(0, 0, 0);
    //startTransform.setOrigin(btVector3(2, 10, 0));
    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);
    pw.dynamicsWorld->addRigidBody(body);
}







class GLDebugDrawer : public btIDebugDraw
{
int m_debugMode = 0;

public:

GLDebugDrawer()
{

}
virtual ~GLDebugDrawer()
{

}

virtual void    drawLine(const btVector3& from,const btVector3& to,const btVector3&  fromColor, const btVector3& toColor)
{
    Shapes3D::DrawLine(glm::make_vec3(&from.getX()), glm::make_vec3(&to.getX()), glm::vec4(fromColor.x(), fromColor.y(), fromColor.z(), 1.0));
}

virtual void    drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
{
    Shapes3D::DrawLine(glm::make_vec3(&from.getX()), glm::make_vec3(&to.getX()), glm::vec4(color.x(), color.y(), color.z(), 1.0));
}

virtual void    drawSphere (const btVector3& p, btScalar radius, const btVector3& color)
{
    Shapes3D::DrawSphere(glm::make_vec3(&p.getX()), radius, glm::vec4(color.x(), color.y(), color.z(), 1.0));
}

virtual void    drawTriangle(const btVector3& a,const btVector3& b,const btVector3& c,const btVector3& color,btScalar alpha)
{
    Shapes3D::DrawTriangle(glm::make_vec3(&a.getX()), glm::make_vec3(&b.getX()), glm::make_vec3(&c.getX()), glm::vec4(color.x(), color.y(), color.z(), alpha));
}

virtual void    drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
{
    Shapes3D::DrawPoint(glm::make_vec3(&PointOnB.getX()), 1.0f, glm::vec4(color.x(), color.y(), color.z(), 1.0));
    drawLine(PointOnB, PointOnB + normalOnB, color);
}

virtual void    reportErrorWarning(const char* warningString)
{
    LOG_WARN("%s", warningString);
}

virtual void    draw3dText(const btVector3& location,const char* textString)
{
    LOG_WARN("unimplemented draw3Dtext bullet debug draw");
}

virtual void    setDebugMode(int debugMode)
{
    this->m_debugMode = debugMode;
}

virtual int     getDebugMode() const { return m_debugMode;}

};

btIDebugDraw* GetTinyPhysicsDebugDrawer(Arena* arena)
{
    GLDebugDrawer* result = (GLDebugDrawer*)arena_alloc(arena, sizeof(GLDebugDrawer));
    new(result) GLDebugDrawer();
    return result;
}