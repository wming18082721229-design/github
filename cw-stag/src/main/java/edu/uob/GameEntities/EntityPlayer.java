package edu.uob.GameEntities;

import java.util.HashMap;

public class EntityPlayer extends EntityCharacter {
    private HashMap<String, EntityArtefact> inventory = new HashMap<>();
    private int health;
    protected EntityLocation currentLocation;

    private final int maxHealth = 3;
    private final EntityLocation startLocation;

    public EntityPlayer(String name, String description, EntityLocation startLocation) {
        super(name, description);
        health = maxHealth;
        this.startLocation = startLocation;
        currentLocation = startLocation;
        currentLocation.addCurrentPlayer(this);
    }

    public EntityLocation getCurrentLocation() {
        return currentLocation;
    }

    public void updateLocation(EntityLocation location) {
        location.removeCurrentPlayer(this);
        this.currentLocation = location;
        currentLocation.addCurrentPlayer(this);
    }

    public HashMap<String, EntityArtefact> getInventory() {
        return this.inventory;
    }

    public EntityArtefact getArtefact(String artefactName) {
        return this.inventory.get(artefactName);
    }

    public void addArtefact(EntityArtefact artefact) {
        inventory.put(artefact.getName(), artefact);
    }

    public EntityArtefact removeArtefact(String artefactName) {
        EntityArtefact artefact = inventory.get(artefactName);
        inventory.remove(artefactName);
        return artefact;
    }

    public int getHealth() {
        return this.health;
    }

    public void increaseHealth() {
        if(this.health == this.maxHealth) return;
        this.health++;
    }

    public void decreaseHealth() {
        if(this.health > 1) this.health--;
        else {
            for(EntityArtefact artefact : inventory.values()) {
                currentLocation.addEntity(artefact);
            }
            inventory.clear();
            this.currentLocation = startLocation;
            this.health = maxHealth;
            System.out.println("you died and lost all of your items, you must return to the start of the game");
        }
    }
}
