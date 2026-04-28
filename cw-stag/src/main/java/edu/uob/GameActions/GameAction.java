package edu.uob.GameActions;

import edu.uob.GameEntities.EntityLocation;
import edu.uob.GameEntities.EntityPlayer;

import java.util.*;

public abstract class GameAction {
    private List<String> triggerList = new ArrayList<>();
    private List<String> subjectList = new ArrayList<>();
    private List<String> consumedList = new ArrayList<>();
    private List<String> producedList = new ArrayList<>();
    private String narration;

    protected HashMap<String, EntityLocation> locationMap;

    public GameAction() {
    }

    public abstract String executeCommand(Set<String> entities, EntityPlayer player);

    public abstract boolean isBuiltin();

    public List<String> getTriggerList() {
        return this.triggerList;
    }

    public void addTrigger(String trigger) {
        if (triggerList.contains(trigger)) {
            System.out.println("[ERROR] trigger: " + trigger + " already exists");
            return;
        }
        this.triggerList.add(trigger);
    }

    public Set<String> getValidEntities() {
        Set<String> validEntities = new HashSet<>(subjectList);
        if (consumedList != null) validEntities.addAll(consumedList);
        if (producedList != null) validEntities.addAll(producedList);
        return validEntities;
    }

    public List<String> getSubjectList() {
        return this.subjectList;
    }

    public void addSubject(String subject) {
        if (subjectList.contains(subject)) {
            System.out.println("[ERROR] subject: " + subject + " already exists");
            return;
        }
        this.subjectList.add(subject);
    }

    public List<String> getConsumedList() {
        return this.consumedList;
    }

    public void addConsumed(String consumed) {
        this.consumedList.add(consumed);
    }

    public List<String> getProducedList() {
        return this.producedList;
    }

    public void addProduced(String produced) {
        this.producedList.add(produced);
    }

    public String getNarration() {
        return this.narration;
    }

    public void setNarration(String narration) {
        this.narration = narration;
    }

    public void setLocationMap(HashMap<String, EntityLocation> locationMap){
        this.locationMap = locationMap;
    }
}
