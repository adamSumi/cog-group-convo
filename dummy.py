import random
from typing import Callable, Literal
import multiprocessing

import rx
from rx import operators as ops
from rx.core.typing import Observer, Scheduler, Observable
from rx.scheduler.threadpoolscheduler import ThreadPoolScheduler

random_juror: Callable[
    [], Literal["juror-a", "juror-b", "juror-c", "jury-foreman", None]
] = lambda: random.choice(["juror-a", "juror-b", "juror-c", "jury-foreman", None])


def infinite_observable(observer: Observer, scheduler: Scheduler) -> None:
    while True:
        val = random_juror()
        print(val)
        observer.on_next(val)


scheduler = ThreadPoolScheduler()
infinite_obs: Observable[
    Literal["juror-a", "juror-b", "juror-c", "jury-foreman", None]
] = rx.create(infinite_observable).pipe(ops.subscribe_on(scheduler))
event_obs: Observable[int] = rx.from_list([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])

rx.combine_latest(infinite_obs, event_obs).pipe(
    # ops.take_until(event_obs.pipe(ops.is_empty()))
).subscribe(on_next=print, on_completed=lambda x: print("All done!"))

while True:
    pass