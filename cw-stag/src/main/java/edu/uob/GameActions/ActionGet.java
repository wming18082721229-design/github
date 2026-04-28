package edu.uob.GameActions;

import edu.uob.GameEntities.EntityArtefact;
import edu.uob.GameEntities.EntityLocation;
import edu.uob.GameEntities.EntityPlayer;

import java.util.Set;

public class ActionGet extends GameAction {
    public ActionGet() {
        super.addTrigger("get");
    }

    public boolean isBuiltin(){
        return true;
    }

    // check if the entity is collectible then add it into player's inventory then remove if from the current location
    public String executeCommand(Set<String> entities, EntityPlayer player) {
        if(entities.size() > 1){
            return  "[ERROR] invalid command: you can get only one artefact each time!\n";
        }else if (entities.isEmpty()) {
            return "[ERROR] invalid command: no subjects are found!\n";
        }
        String artefactName = entities.iterator().next();
        EntityLocation currentLocation = player.getCurrentLocation();
        EntityArtefact artefact = currentLocation.getCurrentArtefacts().get(artefactName);
        if(artefact == null) {
            return "There is no artefact in the current location with the name of \"" + artefactName + "\" !\n";
        }
        player.addArtefact(artefact);
        currentLocation.removeEntity(artefactName);
        return artefactName + " has been put in your inventory!\n";
    }
}
