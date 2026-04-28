package edu.uob.GameActions;

import edu.uob.GameEntities.EntityLocation;
import edu.uob.GameEntities.EntityPlayer;
import edu.uob.GameEntities.GameEntity;

import java.util.ArrayList;
import java.util.Set;

public class ActionLook extends GameAction{
    public ActionLook() {
        super.addTrigger("look");
    }

    public boolean isBuiltin(){
        return true;
    }

    // form then return a String which contains the description of the current location and all entities at it
    public String executeCommand(Set<String> entities, EntityPlayer player) {
        if(!entities.isEmpty()){
            return "[ERROR] invalid command: no subjects are allowed in \"look\" command\n]";
        }
        EntityLocation currentLocation = player.getCurrentLocation();

        StringBuilder outcome = new StringBuilder();
        outcome.append("You are at ").append(currentLocation.getDescription()).append(" now!\n\n");

        ArrayList<GameEntity> playersList = new ArrayList<>(currentLocation.getCurrentPlayers().values());
        descriptionBuilder(outcome, playersList, "players");
        ArrayList<GameEntity> charactersList = new ArrayList<>(currentLocation.getCurrentCharacters().values());
        descriptionBuilder(outcome, charactersList, "characters");
        ArrayList<GameEntity> artefactsList = new ArrayList<>(currentLocation.getCurrentArtefacts().values());
        descriptionBuilder(outcome, artefactsList, "artefacts");
        ArrayList<GameEntity> furnitureList = new ArrayList<>(currentLocation.getCurrentFurniture().values());
        descriptionBuilder(outcome, furnitureList, "furniture");

        ArrayList<GameEntity> toLocationsList = new ArrayList<>(currentLocation.getToLocations().values());
        descriptionBuilder(outcome, toLocationsList, "available path");

        return outcome.toString();
    }

    // append information of the given entityType onto description
    private void descriptionBuilder(StringBuilder description, ArrayList<GameEntity> entitiesList, String entityType) {
        if (entitiesList.isEmpty()){
            description.append("There is no ").append(entityType).append(" in the current location!\n\n");
            return;
        }
        description.append("There are ").append(entityType).append(": \n");
        for(int i  = 0; i < entitiesList.size(); i++) {
            if(i < entitiesList.size() - 1) {
                description.append(entitiesList.get(i).getDescription()).append(", ");
            } else{
                description.append(entitiesList.get(i).getDescription()).append("\nin your current location!\n\n");
            }
        }
    }
}
