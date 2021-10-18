import random
import rx
from rx.core.typing import Observer, Scheduler
from rx import operators as ops
from rx.scheduler.threadpoolscheduler import ThreadPoolScheduler


def random_output(observer: Observer, scheduler: Scheduler) -> None:
    while True:
        observer.on_next(random.randint(0, 1000))
        # print("In loop")


scheduler = ThreadPoolScheduler()
random_obs = rx.create(random_output).pipe(ops.subscribe_on(scheduler))
event_obs = rx.from_list([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
rx.combine_latest(event_obs, random_obs).subscribe(
    on_next=lambda x: print(f"Received {x}")
)
