import threading
import time

cond = threading.Condition()

threshold = 5
num = 0

class Producer(threading.Thread):
    def run(self):
        print("Producer tid: %d" %threading.get_ident())
        while True:
            if cond.acquire():
                if num < threshold:
                    self.Produce()
                else:
                    cond.notify()
                cond.release()

    def Produce(self):
        global num
        print("%d: produce 1" %threading.get_ident())
        num += 1
        time.sleep(0.5)

class Consumer(threading.Thread):
    def run(self):
        print("Consumer tid: %d" %threading.get_ident())
        while True:
            if cond.acquire():
                if num >= threshold:
                    self.Consume()
                else:
                    cond.wait()
                cond.release()

    def Consume(self):
        global num
        print("%d: consume 1" %threading.get_ident())
        num -= 1
        time.sleep(0.5)


producer1 = Producer()
consumer1 = Consumer()
consumer2 = Consumer()

producer1.start()
consumer1.start()
consumer2.start()
while True:
    pass