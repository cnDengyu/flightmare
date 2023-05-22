from microsim import NatNetManager
import time

nat = NatNetManager("192.168.2.100", "192.168.2.100")
nat.connect()
nat.get_description()

for i in range(100):
    print(nat.get_rigidbodies())
    time.sleep(1)

print("Program Finished Normally")
