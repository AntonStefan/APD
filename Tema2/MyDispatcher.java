/* Implement this class. */

import java.util.concurrent.atomic.AtomicInteger;
import java.util.Comparator;
import java.util.List;

public class MyDispatcher extends Dispatcher {

    public MyDispatcher(SchedulingAlgorithm algorithm, List<Host> hosts) {
        super(algorithm, hosts);
    }

    @Override
    public void addTask(Task task) {
        switch (algorithm) {
            case ROUND_ROBIN:
                handleRoundRobin(task);
                break;
            case SHORTEST_QUEUE:
                handleShortestQueue(task);
                break;
            case SIZE_INTERVAL_TASK_ASSIGNMENT:
                handleSITA(task);
                break;
            case LEAST_WORK_LEFT:
                handleLeastWorkLeft(task);
                break;
        }
    }
    private AtomicInteger rrIndex = new AtomicInteger(0); // Index atomic pentru a urmari ultimul nod la care a fost alocat un task

    private void handleRoundRobin(Task task) {
        // Implement Round Robin logic here
        int index = rrIndex.getAndIncrement() % hosts.size();
        hosts.get(index).addTask(task);
    }

    private void handleShortestQueue(Task task) {
        // Implement Shortest Queue logic here
        Host shortestQueueHost = hosts.stream()
                .min(Comparator.comparingInt(Host::getQueueSize))
                .orElse(null);

        if (shortestQueueHost != null) {
            shortestQueueHost.addTask(task);
        }
    }

    private void handleSITA(Task task) {
        // Implement Size Interval Task Assignment logic here
        switch (task.getType()) {
            case SHORT:
                hosts.get(0).addTask(task);
                break;
            case MEDIUM:
                hosts.get(1).addTask(task);
                break;
            case LONG:
                hosts.get(2).addTask(task);
                break;
        }
    }

    private void handleLeastWorkLeft(Task task) {
        // Implement Least Work Left logic here
        Host leastWorkLeftHost = hosts.stream()
                .min(Comparator.comparingLong(Host::getWorkLeft))
                .orElse(null);

        if (leastWorkLeftHost != null) {
            leastWorkLeftHost.addTask(task);
        }
    }
}
