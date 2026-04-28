package edu.uob.GameActions;

import edu.uob.GameEntities.EntityLocation;
import edu.uob.GameEntities.EntityPlayer;

import java.util.Set;

public class ActionHealth extends GameAction{
    public ActionHealth() {
        super.addTrigger("health");
    }

    public boolean isBuiltin(){
        return true;
    }

    // report back the player's current health level
    public String executeCommand(Set<String> entities, EntityPlayer player) {
        if(!entities.isEmpty()){
            return "[ERROR] invalid command: no subjects are allowed in \"health\" command\n]";
        }
        return player.getName() + "'s current health level: " + player.getHealth() + "\n";
    }
}
