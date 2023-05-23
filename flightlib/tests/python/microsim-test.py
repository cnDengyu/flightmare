from microsim import MicroSim, RigidBody
import time, math

# import pycrazyswarm

'''

The recommanded usage of MicroSim is:

    Create client -> Add rigid body -> Connect unity -> Read mocap data -> Send State -> Close client

'''

# [Instruction] Create client
simenv = MicroSim(enable_natnet=False)

# [Instruction] Add rigidbody
simenv.add_rigidbody(10001, RigidBody("cf_test1", [0,0,0.5], [1,0,0,0]))
simenv.add_rigidbody(10002, RigidBody("cf_test2", [1,0,0.5], [1,0,0,0]))

# [Instruction] Connect unity
simenv.connect_unity()

t0 = time.time()
rate = 60
v = 0.1
w = 0.5
theta = 0
while simenv.frame < rate * 1000:

# [Instruction] Read motion capture data
    simenv.natnet_read()

# [Instruction] Send state to unity
    simenv.send_state()

    # simenv.rigidbodies[10001].position[1] += v / rate
    # simenv.rigidbodies[10002].position[1] += v / rate

    simenv.rigidbodies[10001].position[0] = math.cos(theta)
    simenv.rigidbodies[10001].position[1] = math.sin(theta)
    simenv.rigidbodies[10002].position[0] = math.cos(theta+math.pi)
    simenv.rigidbodies[10002].position[1] = math.sin(theta+math.pi)

    theta += w / rate
    
    t1 = time.time()
    if t1 < t0 + 1/rate:
        time.sleep(t0 + 1/rate - t1)
    t0 = t1
    # print(simenv.frame)

# [Instruction] Close client
simenv.close_client()
exit()
