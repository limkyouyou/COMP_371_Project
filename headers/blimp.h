#include <model.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <string>
using namespace std;

class Blimp : public Model {
public:
	float angle = 0.0f;
	float speed = 0.5f;
	float semi_major_axis = 1.0f;	// x axis
	float semi_minor_axis = 2.0f;	// z axis
	vec3 center = vec3(0.0f, 1.5f, 0.0f);

	Blimp(const string& path)
		: Model(path) {}

	void update(float deltaTime) {
		angle += speed * deltaTime;
		if (angle > 2 * pi<float>())
			angle -= 2 * pi<float>();

		position = vec3(
			center.x + semi_major_axis * cos(angle),
			center.y,
			center.z + semi_minor_axis * sin(angle)
		);

		// To make the blimp face the direction it is moving, we need to rotate it so that its
		// forward direction aligns with the tangent of its elliptical path at each frame.
		// -----------------------------------------------------------------------------------
		// 1. Compute tangent vector (derivative of position)
		float dx = -semi_major_axis * sin(angle);
		float dz = semi_minor_axis * cos(angle);
		vec3 dir = normalize(vec3(dx, 0.0f, dz));
		
		// 2. Compute yaw rotation from direction vector
		float yaw = degrees(atan2(dir.x, dir.z));	// yaw = rotation around y-axis

		// 3. Set rotation
		rotation = vec3(0.0f, yaw, 0.0f);
	}
};