package edu.uob.GameActions;

import edu.uob.GameEntities.EntityLocation;
import edu.uob.GameEntities.EntityPlayer;
import edu.uob.GameEntities.GameEntity;

import java.util.List;
import java.util.Set;

public class CustomAction extends GameAction {
    public CustomAction() {}

    public boolean isBuiltin(){
        return false;
    }

    // check availability of consumed and produced then process each of them
    // return narration of this action
    public String executeCommand(Set<String> entities, EntityPlayer player) {
        List<String> consumed = this.getConsumedList();
        List<String> produced = this.getProducedList();
        if(!isConsumable(consumed, player)){
            return consumed + " is not available!";
        }else if(!isProducible(produced)){
            return "[ERROR] " + produced + " cannot be produced! It might have been produced by someone already!";
        }

        for(String c : consumed){
            processConsumed(c, player);
        }
        for(String p : produced){
            processProduced(p, player);}

        return this.getNarration();
    }

    // check if the consumed is available in player's inventory or current location
    private boolean isConsumable(List<String> consumed, EntityPlayer player){
        if(consumed.isEmpty()) return true;
        for(String s : consumed){
            // check every consumed which is not health
            if(!s.equalsIgnoreCase("health")){
                // if a consumed is neither contained in player's inventory/currentLocation,
                // nor one of the reachable location, return false
                if(!player.getInventory().containsKey(s)
                        && !player.getCurrentLocation().getCurrentEntities().containsKey(s)) return false;
            }
        }
        return true;
    }

    // check if the produced is a location or health, otherwise check if it is available in storeroom
    private boolean isProducible(List<String> produced){
        if(produced.isEmpty()) return true;
        for(String s : produced){
            // check every produced which is neither health nor location name
            if(!s.equals("health") && !locationMap.containsKey(s)) {
                // if a produced is not contained in storeroom, return false
                if (!locationMap.get("storeroom").getCurrentEntities().containsKey(s)) return false;
            }

        }
        return true;
    }

    private void processConsumed(String consumed, EntityPlayer player){
        if(consumed == null) return;
        if(consumed.equals("health")) {
            player.decreaseHealth();
            return;
        }

        EntityLocation currentLocation = player.getCurrentLocation();
        GameEntity consumeEntity = currentLocation.removeEntity(consumed);
        // if consumed is a location, remove it from current location's toLocation list only
        if(locationMap.containsKey(consumed)) return;
        else if(consumeEntity == null) consumeEntity = player.removeArtefact(consumed);
        // otherwise add it into storeroom
        locationMap.get("storeroom").addEntity(consumeEntity);
    }

    private void processProduced(String produced, EntityPlayer player){
        if(produced == null) return;
        if(produced.equals("health")) {
            player.increaseHealth();
            return;
        }

        EntityLocation currentLocation = player.getCurrentLocation();
        // if produced is a location, add it into current location's toLocation list only
        if(locationMap.containsKey(produced)) {
            currentLocation.addEntity(locationMap.get(produced));
            return;
        }
        // otherwise remove it form storeroom, then add it into current location
        GameEntity producedEntity = locationMap.get("storeroom").removeEntity(produced);
        currentLocation.addEntity(producedEntity);
    }
}
