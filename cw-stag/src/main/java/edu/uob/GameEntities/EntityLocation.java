package edu.uob.GameEntities;

import java.util.HashMap;

public class EntityLocation extends GameEntity {
    private HashMap<String, EntityPlayer> currentPlayers = new HashMap<>();
    private HashMap<String, EntityCharacter> currentCharacters = new HashMap<>();
    private HashMap<String, EntityArtefact> currentArtefacts = new HashMap<>();
    private HashMap<String, EntityFurniture> currentFurniture = new HashMap<>();
    private HashMap<String, EntityLocation> toLocations = new HashMap<>();

    public EntityLocation(String name, String description) {
        super(name, description);
    }

    public HashMap<String, EntityPlayer> getCurrentPlayers() {
        return currentPlayers;
    }

    public void addCurrentPlayer(EntityPlayer currentPlayers) {
        this.currentPlayers.put(currentPlayers.getName(), currentPlayers);
    }

    public void removeCurrentPlayer(EntityPlayer currentPlayers) {
        this.currentPlayers.remove(currentPlayers.getName());
    }

    // return a HashMap containing this location's characters, artefacts, furniture, and toLocations
    public HashMap<String, GameEntity> getCurrentEntities() {
        HashMap<String, GameEntity> entities = new HashMap<>();
        entities.putAll(currentCharacters);
        entities.putAll(currentArtefacts);
        entities.putAll(currentFurniture);
        entities.putAll(toLocations);
        return entities;
    }

    // remove character/artefact/furniture/toLocation from this location's corresponding map
    public GameEntity removeEntity(String name) {
        if (currentCharacters.containsKey(name)) return currentCharacters.remove(name);
        if (currentArtefacts.containsKey(name))  return currentArtefacts.remove(name);
        if (currentFurniture.containsKey(name))  return currentFurniture.remove(name);
        if (toLocations.containsKey(name))  return toLocations.remove(name);
        return null;
    }

    // put character/artefact/furniture/toLocation into this location's corresponding map
    public void addEntity(GameEntity entity) {
        if (entity instanceof EntityCharacter) currentCharacters.put(entity.getName(), (EntityCharacter) entity);
        else if (entity instanceof EntityArtefact) currentArtefacts.put(entity.getName(), (EntityArtefact) entity);
        else if (entity instanceof EntityFurniture) currentFurniture.put(entity.getName(), (EntityFurniture) entity);
        else if (entity instanceof EntityLocation) toLocations.put(entity.getName(), (EntityLocation) entity);
    }

    public HashMap<String, EntityCharacter> getCurrentCharacters() {
        return this.currentCharacters;
    }

    public HashMap<String, EntityArtefact> getCurrentArtefacts() {
        return this.currentArtefacts;
    }

    public HashMap<String, EntityFurniture> getCurrentFurniture() {
        return this.currentFurniture;
    }

    public HashMap<String, EntityLocation> getToLocations() {
        return this.toLocations;
    }
}
