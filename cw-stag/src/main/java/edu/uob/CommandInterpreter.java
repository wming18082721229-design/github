package edu.uob;

import edu.uob.GameActions.GameAction;
import edu.uob.GameEntities.EntityLocation;
import edu.uob.GameEntities.EntityPlayer;
import edu.uob.GameEntities.GameEntity;

import java.util.*;

public class CommandInterpreter {
    private ConfigLoader configLoader;

    private HashMap<String, List<GameAction>> actionMap;
    private HashMap<String, EntityPlayer> playerMap;

    private EntityPlayer player;
    private Set<String> triggers;
    private Set<String> subjects;

    private GameAction currentAction = null;

    public CommandInterpreter(ConfigLoader configLoader) {
        this.configLoader = configLoader;
        this.playerMap = new HashMap<>();
        this.actionMap = configLoader.getActionMap();
    }

    // try to find corresponding action, configure triggers and subjects for it
    // return true if configured successfully, otherwise print error message
    public boolean interpretCommand(String command) {
        player = null; triggers = new HashSet<>(); subjects = new HashSet<>(); currentAction = null;
        String processedCommand = processCommand(command);
        if (processedCommand == null) {
            System.out.println("[ERROR] no valid player found\n");
            return false;
        } else if (processedCommand.isEmpty()) {
            System.out.println("[ERROR] invalid command\n");
            return false;
        }
        List<String> tokens = Arrays.asList(processedCommand.split("\\s+"));
        if (!findTrigger(tokens)) {
            System.out.println("[ERROR] no triggers found\n");
            return false;
        }
        findEntities(tokens);

        return findAction(triggers, subjects);
    }

    private String processCommand(String command) {
        int colonIndex = command.indexOf(":");
        if (colonIndex == -1) return null;
        String playerName = command.substring(0, colonIndex).trim();
        String cmd = command.substring(colonIndex + 1).trim();
        if(!playerName.matches("^[A-Za-z '\\-]+$")) return null;
        if(playerMap.containsKey(playerName)) player = playerMap.get(playerName);
        else{
            EntityLocation startLocation = configLoader.getStartLocation();
            EntityPlayer newPlayer = new EntityPlayer(playerName,"a player named "+playerName,startLocation);
            playerMap.put(playerName, newPlayer);
            player = newPlayer;
        }

        return cmd.replaceAll("\\p{Punct}", " ").toLowerCase().trim();
    }

    // try to find valid triggers and store them
    // return false if no triggers were found
    private boolean findTrigger(List<String> tokens) {
        // iterate all the possible word length
        for (int phraseLength = tokens.size(); phraseLength > 0; phraseLength--) {
            triggers.addAll(matchPhrases(tokens, phraseLength, actionMap));
        }
        return !triggers.isEmpty();
    }

    // try to find valid subjects and store them
    private void findEntities(List<String> tokens) {
        HashMap<String, GameEntity> allEntities = configLoader.getEntitiesMap();
        // The maximum word length of an entity is N-1
        // iterate all the possible word length
        for (int phraseLength = tokens.size()-1; phraseLength > 0; phraseLength--) {
            subjects.addAll(matchPhrases(tokens, phraseLength, allEntities));
        }
    }

    // iterate all the possible combination of a certain word length
    // return a Set<String> of matched phrases
    private Set<String> matchPhrases(List<String> tokens, int phraseLength, HashMap<String, ?> hashMap) {
        Set<String> result = new HashSet<>();
        for(int i = 0; i <= tokens.size()-phraseLength; i++) {
            String potentialPhrase = String.join(" ", tokens.subList(i, i + phraseLength));
            if (hashMap.containsKey(potentialPhrase))
                result.add(potentialPhrase);
        }
        return result;
    }

    // iterate all the triggers, try to find a sole corresponding action
    // return true and set currentAction, otherwise return false and print error message
    private boolean findAction(Set<String> triggers, Set<String> subjects){
        Set<GameAction> potentialActions = new HashSet<>();
        for(String trigger : triggers){
            Set<GameAction> tempActions = actionFilter(trigger, subjects);
            if(!tempActions.isEmpty()) potentialActions.addAll(tempActions);
        }
        if(potentialActions.size() > 1){
            if(triggers.size() > 1){
                System.out.println("[ERROR] command involving more than one actions is NOT supported.\n");
            } else {
                System.out.println("there is more than one action possible - which one do you want to perform?\n");
            }
            return false;
        } else if(potentialActions.isEmpty()){
            System.out.println("[ERROR] attempting to perform actions with inappropriate subjects.\n");
            return false;
        } else currentAction = potentialActions.iterator().next();
        return true;
    }

    // iterate all the actions referred by a certain trigger
    // return a set of valid GameAction
    private Set<GameAction> actionFilter(String trigger, Set<String> subjects){
        Set<GameAction> result = new HashSet<>(actionMap.get(trigger));
        Set<GameAction> toRemove = new HashSet<>();
        for(GameAction action : result){
            if(action.isBuiltin()) continue;
            for(String subject : subjects){
                // what about something occurs in <consumed>/<produced> but not in <subjects>?
                if (!action.getValidEntities().contains(subject)) {
                    toRemove.add(action);
                    break;
                }
            }
        }
        result.removeAll(toRemove);
        return result;
    }

    // execute command with corresponding action
    public String executeCommand() {
        return currentAction.executeCommand(subjects, player);
    }
    // case-insensitive
    // decorative words cannot be included in-between the triggers phrase words
    // order of action triggers words must not differ from that specified in the actions file
    // support variable whitespace
    // MUST contain a triggers phrase and at least one subjects
    // subjects not specified in the action file should prevent a match from being made
    // specifying extraneous subjects for built-in command (e.g. get key from forest) should not be permitted

    // If a particular command is ambiguous (i.e. there is more than one valid and performable action possible)
    // then NO action should be performed and a suitable warning message sent back to the user
    // (e.g. there is more than one 'open' action possible - which one do you want to perform ?)

    // A single command can only be used to perform a single built-in command or single custom game action.
    // Users are unable to use commands such as get axe and coin, get key and open door or open door and potion.
}

