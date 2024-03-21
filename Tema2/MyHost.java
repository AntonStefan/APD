/* Implement this class. */

import java.util.Comparator;
import java.util.LinkedList;
import java.util.PriorityQueue;
import java.util.Queue;


public class MyHost extends Host {
    // Define a comparator for the PriorityQueue
    Comparator<Task> priorityComparator = (task1, task2) -> Integer.compare(task2.getPriority(), task1.getPriority());

    private final Queue<Task> taskQueue = new PriorityQueue<>(priorityComparator);
    private final Object queueLock = new Object();
    private Task currentTask = null;
    private volatile boolean  isTrue = false;
    private boolean isPre = false;
    private long taskStartTime = 0;  // Start time of the current task
    private long endTime = 0;  // End time of the current task
    private final Object Locker = new Object();


    @Override
    public void run() {
        while (true) {
            if(isTrue && taskQueue.isEmpty()) {
                return;
            }
            synchronized (queueLock) {
                if(currentTask == null) {
                    currentTask = taskQueue.poll();
                }
            }
            if (currentTask != null ) {

                long taskDuration = currentTask.getLeft();
                taskStartTime = System.currentTimeMillis();  // Capture start time
                endTime = System.currentTimeMillis() + currentTask.getLeft();
                synchronized (Locker) {
                    // Simuleaza executarea task-ului
                    try {
                        Thread.sleep(taskDuration);
                        long elapsedTime = System.currentTimeMillis() - taskStartTime;
                        currentTask.setLeft(taskDuration - elapsedTime);
                        if (currentTask.getLeft() < 0) {
                            currentTask.finish();
                            currentTask = null;
                        }

                    } catch(InterruptedException e){
                        // Task was interrupted
                        long elapsedTime = System.currentTimeMillis() - taskStartTime;

                        currentTask.setLeft(taskDuration - elapsedTime);  // Update remaining
                        if (currentTask.getLeft() < 0) {
                            currentTask.finish();
                            currentTask = null;
                        } else {
                            if(isPre) {
                                taskQueue.add(currentTask);

                                currentTask = null;
                            }
                        }
                        isPre = false;
                    }
                }
            }
        }
    }


    @Override
    public void addTask(Task task) {
        synchronized (queueLock) {  // era queueLock
            taskQueue.add(task);

            if (currentTask != null && currentTask.isPreemptible() && task.getPriority() > currentTask.getPriority()) {
                isPre = true;  // Interrupt the running task
                interrupt();
            }
        }
    }

    @Override
    public int getQueueSize() {
        if(currentTask != null) {
            return taskQueue.size() + 1;
        }
        return taskQueue.size();
    }

    @Override
    public long getWorkLeft() {
        interrupt();
        //synchronized (Locker) {
        long workLeft = 0;
        for (Task task : taskQueue) {
            workLeft += task.getLeft();
        }
        if (currentTask != null) {
            workLeft += currentTask.getLeft();
        }
        return workLeft;

    }

    @Override
    public void shutdown() {
        isTrue = true;
    }
}
