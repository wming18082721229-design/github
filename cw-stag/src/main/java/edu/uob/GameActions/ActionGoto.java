package edu.uob.GameActions;

import edu.uob.GameEntities.EntityLocation;
import edu.uob.GameEntities.EntityPlayer;

import java.util.Set;

public class ActionGoto extends GameAction{
    public ActionGoto() {
        super.addTrigger("goto");
    }

    public boolean isBuiltin(){
        return true;
    }

    // check if the toLocation is reachable then change the player's current location
    public String executeCommand(Set<String> entities, EntityPlayer player) {
        if(entities.size() > 1){
            return  "[ERROR] invalid command: you can go to only one location each time!\n";
        }else if (entities.isEmpty()) {
            return "[ERROR] invalid command: no subjects are found!\n";
        }
        String locationName = entities.iterator().next();
        EntityLocation currentLocation = player.getCurrentLocation();
        EntityLocation toLocation = currentLocation.getToLocations().get(locationName);
        if(toLocation == null) {
            return "There is no path to \"" + locationName + "\" in your current location!\n";
        }
        player.updateLocation(toLocation);
        return "You have successfully arrived at " + locationName + " !\n";
    }
}
