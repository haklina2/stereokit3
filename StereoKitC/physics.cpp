#pragma comment(lib, "reactphysics3d.lib")

#include "physics.h"
#include "stereokit.h"
#include "_stereokit.h"

#include <vector>
using namespace std;
#include <reactphysics3d.h>
#include <constraint/ContactPoint.h>
#include <collision/ContactManifold.h>
using namespace reactphysics3d;

struct solid_move_t {
	RigidBody *body;
	Vector3    dest;
	Quaternion dest_rot;
	Vector3    old_velocity;
	Vector3    old_rot_velocity;
};
vector<solid_move_t> solid_moves;

struct physics_shape_asset_t {
	int64_t id;
	void   *shape;
	int     refs;
};

double physics_sim_time = 0;
double physics_step = 1 / 90.0;

DynamicsWorld *physics_world;

vector<physics_shape_asset_t> physics_shapes;
vector<vec3>                  physics_contacts;
vector<solid_t>               physics_solids;

void physics_init() {

	WorldSettings settings;
	physics_world = new DynamicsWorld(Vector3(0,-9.81f,0), settings);
}

void physics_shutdown() {
	delete physics_world;
}

void physics_update() {
	// How many physics frames are we going to be calculating this time?
	int frames = ceil((sk_timev - physics_sim_time) / physics_step);
	if (frames <= 0)
		return;

	// Calculate move velocities for objects that need to be at their destination by the end of this function!
	for (size_t i = 0; i < solid_moves.size(); i++) {
		solid_move_t &move = solid_moves[i];
		// Position
		move.old_velocity  = move.body->getLinearVelocity();
		Vector3       pos  = move.body->getTransform().getPosition();
		Vector3       velocity = (move.dest - pos) / (physics_step * frames);
		move.body->setLinearVelocity(velocity);
		// Rotation
		move.old_rot_velocity = move.body->getAngularVelocity();
		Quaternion rot   = move.body->getTransform().getOrientation();
		if (rot.x != move.dest_rot.x || rot.y != move.dest_rot.y || rot.z != move.dest_rot.z || rot.w != move.dest_rot.w) {
			Quaternion delta = move.dest_rot * rot.getInverse();
			float   angle;
			Vector3 axis;
			delta.getRotationAngleAxis(angle, axis);
			if (!isnan(angle)) {
				move.body->setAngularVelocity((angle / (physics_step * frames)) * axis.getUnit());
			}
		}
	}

	// Sim physics!
	while (physics_sim_time < sk_timev) {
		physics_world->update(physics_step);
		physics_sim_time += physics_step;
	}

	// Reset moved objects back to their original values, and clear out our list
	for (size_t i = 0; i < solid_moves.size(); i++) {
		solid_moves[i].body->setLinearVelocity (solid_moves[i].old_velocity);
		solid_moves[i].body->setAngularVelocity(solid_moves[i].old_rot_velocity);
	}
	solid_moves.clear();

	physics_show_colliders();
}

mesh_t     debug_box = nullptr;
mesh_t     debug_sphere = nullptr;
material_t debug_mat = nullptr;
material_t debug_mat_ct = nullptr;
material_t debug_mat_sl = nullptr;
void physics_show_colliders() {
	return;
	if (debug_box    == nullptr) debug_box    = mesh_gen_cube  ("physics/debug_cube", vec3{ 1,1,1 });
	if (debug_sphere == nullptr) debug_sphere = mesh_gen_sphere("physics/debug_sphere", 1, 2);
	if (debug_mat    == nullptr) {
		shader_t shader = shader_find("default/shader");
		debug_mat = material_create("physics/debug_material", shader);
		material_set_alpha_mode(debug_mat, material_alpha_blend);
		material_set_color32(debug_mat, "color", { 100, 200, 100, 100 });
		shader_release(shader);
	}
	if (debug_mat_ct    == nullptr) {
		shader_t shader = shader_find("default/shader");
		debug_mat_ct = material_create("physics/debug_material_ct", shader);
		material_set_alpha_mode(debug_mat_ct, material_alpha_blend);
		material_set_color32(debug_mat_ct, "color", { 200, 100, 100, 100 });
		shader_release(shader);
	}
	if (debug_mat_sl    == nullptr) {
		shader_t shader = shader_find("default/shader");
		debug_mat_sl = material_create("physics/debug_material_sl", shader);
		material_set_alpha_mode(debug_mat_sl, material_alpha_blend);
		material_set_color32(debug_mat_sl, "color", { 200, 200, 100, 100 });
		shader_release(shader);
	}

	transform_t tr;
	for (size_t i = 0; i < physics_solids.size(); i++) {
		RigidBody        *body  = (RigidBody*)physics_solids[i];
		const ProxyShape *shape = body->getProxyShapesList();
		if (!body->isActive())
			continue;
		while (shape != nullptr) {
			const CollisionShape *asset = shape->getCollisionShape();
			CollisionShapeName    name  = asset->getName();
			transform_set(tr,
				(const vec3&)shape->getLocalToWorldTransform().getPosition(),
				{ 1,1,1 },
				(const quat&)shape->getLocalToWorldTransform().getOrientation());

			switch (name) {
			case CollisionShapeName::BOX: {
				BoxShape *box = (BoxShape*)asset;
				transform_set_scale(tr, (const vec3 &)(box->getExtent()*2.01f));
				render_add_mesh(debug_box, body->isSleeping() ? debug_mat_sl : debug_mat, tr);
			} break;
			case CollisionShapeName::SPHERE: {
				SphereShape *sphere = (SphereShape*)asset;
				transform_set_scale(tr, vec3{ 1,1,1 }*sphere->getRadius()*2.01f);
				render_add_mesh(debug_sphere, body->isSleeping() ? debug_mat_sl : debug_mat, tr);
			} break;
			case CollisionShapeName::CAPSULE: {
				CapsuleShape *capsule = (CapsuleShape*)asset;
				transform_set_scale(tr, vec3{ 1,1,1 }*capsule->getRadius()*2.01f);
				render_add_mesh(debug_sphere, body->isSleeping() ? debug_mat_sl : debug_mat, tr);
			} break;
			}
			shape = shape->getNext();
		}
	}

	transform_set(tr, { 0,0,0 }, { 0.02f,0.02f,0.02f }, { 0,0,0,1 });
	List<const ContactManifold *> contacts = physics_world->getContactsList();
	for (size_t i = 0; i < contacts.size(); i++) {
		ContactPoint* cts = contacts[i]->getContactPoints();
		while (cts != nullptr) {
			Vector3 pt = contacts[i]->getShape1()->getLocalToWorldTransform() * cts->getLocalPointOnShape1();
			transform_set_position(tr, { pt.x, pt.y, pt.z });
			render_add_mesh(debug_box, debug_mat_ct, tr);
			cts = cts->getNext();
		}
	}
}

class get_at_RaycastCallback : RaycastCallback { public: solid_t result=nullptr; Vector3 start; float min = 100; decimal notifyRaycastHit(const RaycastInfo &raycastInfo) {
	float dist = (raycastInfo.worldPoint - start).lengthSquare();
	if (dist < min) {
		min    = dist;
		result = raycastInfo.body;
	}
	return 1;
} };
solid_t physics_get_at(vec3 pt) {
	Vector3 p = Vector3(pt.x, pt.y, pt.z);
	
	get_at_RaycastCallback cb;
	cb.start = p;
	physics_world->raycast(Ray(p+Vector3{ 0, 1, 0 }, p), (RaycastCallback*)&cb);
	if (cb.result != nullptr) {
		log_writef(log_info, "%.2f", cb.min);
	}
	if (cb.result != nullptr)
		return cb.result;
	return nullptr;
}

solid_t solid_create(const vec3 &position, const quat &rotation, solid_type_ type) {
	RigidBody *body = physics_world->createRigidBody(Transform((Vector3 &)position, (Quaternion &)rotation));
	solid_set_type(body, type);
	physics_solids.push_back(body);
	return (solid_t)body;
}
void solid_release(solid_t solid) {
	RigidBody        *body  = (RigidBody*)solid;
	const ProxyShape *shape = body->getProxyShapesList();
	while (shape != nullptr) {
		const CollisionShape *asset = shape->getCollisionShape();

		CollisionShapeName name = asset->getName();
		if (name == CollisionShapeName::BOX || name == CollisionShapeName::SPHERE || name == CollisionShapeName::CAPSULE)
			delete asset;
		else
			log_write(log_warning, "Haven't added support for all physics shapes yet!");

		shape = shape->getNext();
	}

	physics_world->destroyRigidBody((RigidBody *)solid);

	vector<solid_t>::iterator item = find(physics_solids.begin(), physics_solids.end(), solid);
	if (item != physics_solids.end())
		physics_solids.erase(item);
}

void solid_add_sphere(solid_t solid, float diameter, float kilograms, const vec3 *offset) {
	RigidBody   *body   = (RigidBody*)solid;
	SphereShape *sphere = new SphereShape(diameter/2);
	body->addCollisionShape(sphere, Transform(offset == nullptr ? Vector3(0,0,0) : (Vector3 &)*offset, { 0,0,0,1 }), kilograms);
}

void solid_add_box(solid_t solid, const vec3 &dimensions, float kilograms, const vec3 *offset) {
	RigidBody *body = (RigidBody*)solid;
	BoxShape  *box  = new BoxShape((Vector3&)(dimensions/2));
	body->addCollisionShape(box, Transform(offset == nullptr ? Vector3(0,0,0) : (Vector3 &)*offset, { 0,0,0,1 }), kilograms);
}

void solid_add_capsule(solid_t solid, float diameter, float height, float kilograms, const vec3 *offset) {
	RigidBody    *body    = (RigidBody*)solid;
	CapsuleShape *capsule = new CapsuleShape(diameter/2, height);
	body->addCollisionShape(capsule, Transform(offset == nullptr ? Vector3(0,0,0) : (Vector3 &)*offset, { 0,0,0,1 }), kilograms);
}

void solid_add_joint(solid_t solid_a, solid_t solid_b) {
	RigidBody *a = (RigidBody*)solid_a, *b = (RigidBody*)solid_b;

	const Vector3 anchorPoint = rp3d::decimal(0.5) * (a->getTransform().getPosition() + b->getTransform().getPosition()); 

	// Slider axis in world-space 
	const Vector3 axis = (b->getTransform().getPosition() - a->getTransform().getPosition() ); 

	// Create the joint info object 
	SliderJointInfo jointInfo(a, b, anchorPoint, axis);
	jointInfo.isLimitEnabled = true;
	jointInfo.minTranslationLimit = -axis.length()/2;
	jointInfo.maxTranslationLimit = 0.001f;

	jointInfo.isMotorEnabled = true;
	jointInfo.motorSpeed     = axis.length() * 20;
	jointInfo.maxMotorForce  = 100;
	jointInfo.isCollisionEnabled = true;

	// Create the slider joint in the dynamics world 
	SliderJoint* joint; 
	joint = dynamic_cast<SliderJoint*>(physics_world->createJoint(jointInfo));
}
void solid_add_joint2(solid_t solid_a, solid_t solid_b) {
	RigidBody *a = (RigidBody*)solid_a, *b = (RigidBody*)solid_b;

	const Vector3 anchorPoint = rp3d::decimal(0.5) * (a->getTransform().getPosition() + b->getTransform().getPosition());

	// Slider axis in world-space 
	Vector3 axis = (b->getTransform().getPosition() - a->getTransform().getPosition()); 
	axis.normalize();

	// Create the joint info object 
	HingeJointInfo jointInfo(a, b, anchorPoint, axis);

	//jointInfo.isLimitEnabled = true;
	//jointInfo.
	//jointInfo.minTranslationLimit = -axis.length()/2;
	//jointInfo.maxTranslationLimit = 0.001f;

	//jointInfo.isMotorEnabled = true;
	//jointInfo.motorSpeed     = axis.length() * 20;
	//jointInfo.maxMotorForce  = 100;
	//jointInfo.isCollisionEnabled = true;

	// Create the slider joint in the dynamics world 
	HingeJoint* joint; 
	//joint = dynamic_cast<HingeJoint*>(physics_world->createJoint(jointInfo));
}
joint_t solid_add_joint3(solid_t solid_a, solid_t solid_b) {
	RigidBody *a = (RigidBody*)solid_a, *b = (RigidBody*)solid_b;

	const Vector3 anchorPoint = rp3d::decimal(0.5) * (a->getTransform().getPosition() + b->getTransform().getPosition());

	FixedJointInfo jointInfo(a,b,anchorPoint);
	
	FixedJoint *joint = dynamic_cast<FixedJoint*>(physics_world->createJoint(jointInfo));
	return joint;
}
void joint_destroy(joint_t joint) {
	Joint *j = (Joint *)joint;
	physics_world->destroyJoint(j);
}

void solid_set_type(solid_t solid, solid_type_ type) {
	RigidBody *body = (RigidBody *)solid;

	switch (type) {
	case solid_type_normal:     body->setType(BodyType::DYNAMIC);   break;
	case solid_type_immovable:  body->setType(BodyType::STATIC);    break;
	case solid_type_unaffected: body->setType(BodyType::KINEMATIC); break;
	}
}

void solid_set_enabled(solid_t solid, bool32_t enabled) {
	RigidBody *body = (RigidBody *)solid;
	body->setIsActive(enabled);
}

void solid_set_gravity(solid_t solid, bool32_t enabled) {
	RigidBody *body = (RigidBody *)solid;
	body->enableGravity(enabled);
}

void solid_teleport(solid_t solid, const vec3 &position, const quat &rotation) {
	RigidBody *body = (RigidBody *)solid;Transform t = Transform((Vector3 &)position, (Quaternion &)rotation);
	body->setTransform(t);
	body->setIsSleeping(false);
}

void solid_move(solid_t solid, const vec3 &position, const quat &rotation) {
	RigidBody *body = (RigidBody *)solid;
	solid_moves.push_back(solid_move_t{body, Vector3(position.x, position.y, position.z), Quaternion(rotation.i, rotation.j, rotation.k, rotation.a)});
}

void solid_set_velocity(solid_t solid, const vec3 &meters_per_second) {
	RigidBody *body = (RigidBody *)solid;
	body->setLinearVelocity((Vector3&)meters_per_second);
}
void solid_set_velocity_ang(solid_t solid, const vec3 &radians_per_second) {
	RigidBody *body = (RigidBody *)solid;
	body->setAngularVelocity((Vector3&)radians_per_second);
}

void solid_get_transform(const solid_t solid, transform_t &out_transform) {
	const Transform &solid_tr = ((RigidBody *)solid)->getTransform();
	memcpy(&out_transform._position, &solid_tr.getPosition   ().x, sizeof(vec3));
	memcpy(&out_transform._rotation, &solid_tr.getOrientation().x, sizeof(quat));
	out_transform._dirty = true;
}