package edu.uob;

import com.alexmerz.graphviz.ParseException;
import com.alexmerz.graphviz.Parser;
import com.alexmerz.graphviz.objects.Edge;
import com.alexmerz.graphviz.objects.Graph;
import com.alexmerz.graphviz.objects.Node;
import edu.uob.GameActions.*;
import edu.uob.GameEntities.*;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;

public class ConfigLoader {
    private EntityLocation startLocation = null;

    private HashMap<String, EntityLocation> locationMap = new HashMap<>();
    private HashMap<String, List<GameAction>> actionMap = new HashMap<>();
    private HashMap<String, GameEntity> entitiesMap = new HashMap<>();

    public ConfigLoader(File entitiesFile, File actionsFile) {
        loadConfig(entitiesFile, actionsFile);
    }

    private void loadConfig(File entitiesFile, File actionsFile) {
        parseEntities(entitiesFile);
        initializeBuiltinActions();
        parseActions(actionsFile);
    }

    // parse .dot file, initialize locations and entities in it
    public void parseEntities(File entitiesFile){
        try {
            Parser parser = new Parser();
            FileReader reader = new FileReader(entitiesFile);
            parser.parse(reader);
            Graph wholeDocument = parser.getGraphs().get(0);
            ArrayList<Graph> sections = wholeDocument.getSubgraphs();
            ArrayList<Graph> locations = sections.get(0).getSubgraphs();
            ArrayList<Edge> paths = sections.get(1).getEdges();
            for(Graph location : locations){
                initializeLocation(location);
            }
            entitiesMap.putAll(locationMap);
            for(Edge path : paths){
                initializePath(path);
            }
        } catch (FileNotFoundException fnfe) {
            System.err.println("File not found");
        } catch (ParseException pe) {
            System.err.println("Error parsing file");
        }
    }

    // parse .xml file, build HashMap<trigger, GameAction>
    private void parseActions(File actionsFile){
        try {
            DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
            Document document = builder.parse(actionsFile);
            Element root = document.getDocumentElement();
            NodeList actions = root.getElementsByTagName("action");
            // Get the action (only the odd items are actually actions - 1, 3, 5 etc.)
            for(int i = 0; i < actions.getLength(); i++){
                Element actionElement = (Element)actions.item(i);
                initializeAction(actionElement);
            }
        } catch(ParserConfigurationException pce) {
            System.err.println("Error parsing file: ParserConfigurationException");
        } catch(SAXException saxe) {
            System.err.println("Error parsing file: SAXException");
        } catch(IOException ioe) {
            System.err.println("Error parsing file:  IOException");
        }
    }

    // create a location with name and description, then register it in locationMap
    // register all entities that belong to this particular location
    private void initializeLocation(Graph location) {
        Node locationDetails = location.getNodes(true).get(0);
        String locationName = locationDetails.getId().getId();
        String locationDescription = locationDetails.getAttribute("description");
        EntityLocation newLocationEntity = new EntityLocation(locationName, locationDescription);
        // subgraphs of the very location graph are graphs of entities(other than EntityLocation)
        ArrayList<Graph> entityGraphs = location.getSubgraphs();
        for(Graph entityGraph: entityGraphs){
            String entityType = entityGraph.getId().getId();
            ArrayList<Node> entitiesDetails = entityGraph.getNodes(false);
            for(Node entityDetails: entitiesDetails){
                initializeEntity(newLocationEntity, entityType, entityDetails);
            }
        }
        if(startLocation == null) startLocation = newLocationEntity;
        locationMap.put(locationName, newLocationEntity);
        entitiesMap.putAll(newLocationEntity.getCurrentCharacters());
        entitiesMap.putAll(newLocationEntity.getCurrentArtefacts());
        entitiesMap.putAll(newLocationEntity.getCurrentFurniture());
    }

    // create an entity with name and description, then register it in location's corresponding HashMap
    private void initializeEntity(EntityLocation location, String entityType, Node entityDetails) {
        String name = entityDetails.getId().getId();
        String description = entityDetails.getAttribute("description");
        switch (entityType.toLowerCase()) {
            case "characters" -> location.addEntity(new EntityCharacter(name, description));
            case "artefacts"  -> location.addEntity(new EntityArtefact(name, description));
            case "furniture"  -> location.addEntity(new EntityFurniture(name, description));
            default -> throw new IllegalArgumentException("Unknown entity type: " + entityType);
        }
    }

    // initialize path, register the toLocation in fromLocation's HashMap
    private void initializePath(Edge path){
        Node fromLocation = path.getSource().getNode();
        String fromName = fromLocation.getId().getId();
        EntityLocation fromLocationEntity = locationMap.get(fromName);
        Node toLocation = path.getTarget().getNode();
        String toName = toLocation.getId().getId();
        EntityLocation toLocationEntity = locationMap.get(toName);
        if(fromLocationEntity == null || toLocationEntity == null
                || fromName.equals(toName) || fromName.equals("storeroom") || toName.equals("storeroom"))
        {
            System.out.println("Invalid path: " + fromName + " -> " + toName);
            return;
        }
        fromLocationEntity.addEntity(toLocationEntity);
    }

    // register all built-in actions
    private void initializeBuiltinActions(){
        registerAction(new ActionInv());
        registerAction(new ActionGet());
        registerAction(new ActionDrop());
        registerAction(new ActionGoto());
        registerAction(new ActionLook());
        registerAction(new ActionHealth());
    }

    // create a custom action with its triggers, subjects, consumed, produced, and narration, then register it
    private void initializeAction(Element actionElement){
        GameAction newAction = new CustomAction();
        NodeList elementsList = actionElement.getChildNodes(); // {triggers, subjects, consumed, produced}
        for(int i = 0; i < elementsList.getLength(); i++){
            if (elementsList.item(i).getNodeType() != 1) continue;
            Element phrasesGroup = (Element)elementsList.item(i); // refers to phrases of triggers/subjects/...
            String phraseType = phrasesGroup.getTagName();
            if (phraseType.equalsIgnoreCase("narration")) {
                newAction.setNarration(phrasesGroup.getTextContent().trim());
                continue;
            }
            NodeList phraseList = phrasesGroup.getChildNodes(); // phrases of a certain phrase group
            for(int j = 0; j < phraseList.getLength(); j++){
                if (phraseList.item(j).getNodeType() != 1) continue;
                String phrase = phraseList.item(j).getTextContent().trim();
                if (phraseType.equalsIgnoreCase("triggers")) {
                    newAction.addTrigger(phrase);
                } else if (phrase.equalsIgnoreCase("health") || entitiesMap.containsKey(phrase)) {
                    switch (phraseType.toLowerCase()) {
                        case "subjects" -> newAction.addSubject(phrase);
                        case "consumed" -> newAction.addConsumed(phrase);
                        case "produced" -> newAction.addProduced(phrase);
                        default -> throw new IllegalArgumentException("Unknown phrase type: " + phraseType);
                    }
                }
            }
        }
        newAction.setLocationMap(locationMap);
        registerAction(newAction);
    }

    // build map between triggers and its action
    private void registerAction(GameAction action) {
        for (String trigger : action.getTriggerList()) {
            if (!actionMap.containsKey(trigger)) {
                actionMap.put(trigger, new ArrayList<>());
            }
            actionMap.get(trigger).add(action);
        }
    }
        /*
        // triggers
        Element triggers = (Element)actionElement.getElementsByTagName("triggers").item(0);
        for(int i=0; i < triggers.getElementsByTagName("keyphrase").getLength(); i++){
            String triggerPhrase = triggers.getElementsByTagName("keyphrase").item(i).getTextContent();
            newAction.addTrigger(triggerPhrase);
        }
        // subjects
        Element subjects = (Element)actionElement.getElementsByTagName("subjects").item(0);
        for(int i=0; i < subjects.getElementsByTagName("entity").getLength(); i++){
            String subjectPhrase = subjects.getElementsByTagName("entity").item(i).getTextContent();
            newAction.addSubject(subjectPhrase);
        }
        // consumed
        Element consumed = (Element)actionElement.getElementsByTagName("consumed").item(0);
        for(int i=0; i < consumed.getElementsByTagName("entity").getLength(); i++){
            String consumedPhrase = triggers.getElementsByTagName("entity").item(i).getTextContent();
            newAction.addConsumed(consumedPhrase);
        }
        // produced
        Element produced = (Element)actionElement.getElementsByTagName("produced").item(0);
        for(int i=0; i < produced.getElementsByTagName("entity").getLength(); i++){
            String producedPhrase = produced.getElementsByTagName("entity").item(i).getTextContent();
            newAction.addProduced(producedPhrase);
        }
        // narration
        String narration = actionElement.getElementsByTagName("narration").item(0).getTextContent();
        newAction.setNarration(narration);

         */

    public HashMap<String, EntityLocation> getLocationMap(){
        return this.locationMap;
    }

    public EntityLocation getStartLocation(){
        return this.startLocation;
    }

    public HashMap<String, List<GameAction>> getActionMap(){
        return this.actionMap;
    }

    public HashMap<String, GameEntity> getEntitiesMap(){
        return this.entitiesMap;
    }
}
