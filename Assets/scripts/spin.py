from golias import *


class Spin(PythonBehavior):

    speed = 90.0

    def start(self):
        print("Spin.start on", self.get_name())
        self.transform.set_position(Vector3(-2.0, 2.5, 1.0))
        

    def update(self, delta_time):
        self.transform.rotate_local(Vector3(0.0, 1.0, 0.5), deg_to_rad(self.speed * delta_time))

        pos = Input.get_mouse_position()
        print(f"Spin.update cursor position: Vector2({pos.x}, {pos.y})")
