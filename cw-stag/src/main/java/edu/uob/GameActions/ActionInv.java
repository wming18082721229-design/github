package edu.uob.GameActions;

import edu.uob.GameEntities.EntityArtefact;
import edu.uob.GameEntities.EntityPlayer;

import java.util.ArrayList;
import java.util.Set;

public class ActionInv extends GameAction{
    public ActionInv() {
        super.addTrigger("inventory");
        super.addTrigger("inv");
    }

    public boolean isBuiltin(){
        return true;
    }

    // return information of entities in player's inventory
    public String executeCommand(Set<String> entities, EntityPlayer player) {
        if(!entities.isEmpty()){
            return "[ERROR] invalid command: no subjects are allowed in \"inventory\" command\n]";
        }
        ArrayList<EntityArtefact> artefactsList = new ArrayList<>(player.getInventory().values());
        if(artefactsList.isEmpty()){
            return "You have nothing in your inventory!\n";
        }
        StringBuilder outcome = new StringBuilder();
        for(EntityArtefact artefact: artefactsList) {
            outcome.append(artefact.getName()).append(", ");
        }
        return "You have \"" + outcome.substring(0,outcome.length()-2) + "\" in your inventory!\n";
    }
}
