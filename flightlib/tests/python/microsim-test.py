from microsim import MicroSim, RigidBody
import time

simenv = MicroSim(enable_natnet=False)

simenv.add_rigidbody(0, RigidBody("cf_test1", [0,0,0.5], [1,0,0,0]))
simenv.add_rigidbody(1, RigidBody("cf_test2", [1,0,0.5], [1,0,0,0]))

simenv.connect_unity()

t0 = time.time()
rate = 60
v = 1
while simenv.frame < rate*10:
    simenv.sendState()
    simenv.rigidbodies[0].position[1] += v / rate
    simenv.rigidbodies[1].position[1] += v / rate
    
    t1 = time.time()
    if t1 < t0 + 1/rate:
        time.sleep(t0 + 1/rate - t1)
    t0 = t1

simenv.close_client()
