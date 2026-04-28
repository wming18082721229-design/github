package edu.uob.GameActions;

import edu.uob.GameEntities.EntityArtefact;
import edu.uob.GameEntities.EntityLocation;
import edu.uob.GameEntities.EntityPlayer;
import edu.uob.GameEntities.GameEntity;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

public class ActionDrop extends GameAction{
    public ActionDrop() {
        super.addTrigger("drop");
    }

    public boolean isBuiltin(){
        return true;
    }

    // check if the entity is available then add it into current location then remove if from the player's inventory
    public String executeCommand(Set<String> entities, EntityPlayer player) {
        if(entities.size() > 1){
            return  "[ERROR] invalid command: you can drop only one artefact each time!\n";
        }else if (entities.isEmpty()) {
            return "[ERROR] invalid command: no subjects are found!\n";
        }
        String artefactName = entities.iterator().next();
        EntityArtefact artefact = player.getArtefact(artefactName);
        if(artefact == null) {
            return "There is no artefact in your inventory with the name of \"" + artefactName + "\" !\n";
        }
        EntityLocation currentLocation = player.getCurrentLocation();
        currentLocation.addEntity(artefact);
        player.removeArtefact(artefactName);
        return artefactName + " has been dropped at " + currentLocation.getName() + " !\n";
    }
}
