package edu.uob;

import static org.junit.jupiter.api.Assertions.assertTimeoutPreemptively;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import java.io.File;
import java.nio.file.Paths;
import java.time.Duration;

class ExtendedSTAGTests {

    private GameServer server;
    private ConfigLoader configLoader;
    private CommandInterpreter commandInterpreter;

    // Create a new server _before_ every @Test
    @BeforeEach
    void setup() {
        File entitiesFile = Paths.get("config" + File.separator + "extended-entities.dot").toAbsolutePath().toFile();
        File actionsFile = Paths.get("config" + File.separator + "extended-actions.xml").toAbsolutePath().toFile();
        server = new GameServer(entitiesFile, actionsFile);
        configLoader = server.getConfigLoader();
        commandInterpreter = server.getCommandInterpreter();
    }

    String sendCommandToServer(String command) {
        // Try to send a command to the server - this call will timeout if it takes too long (in case the server enters an infinite loop)
        return assertTimeoutPreemptively(Duration.ofMillis(1000), () -> { return server.handleCommand(command);},
                "Server took too long to respond (probably stuck in an infinite loop)");
    }

    void cabinToCellar(){
        sendCommandToServer("simon: goto forest");
        sendCommandToServer("simon: get the key");
        sendCommandToServer("simon: goto cabin");
        // unlock the door
        sendCommandToServer("simon: open the trapdoor");
        sendCommandToServer("simon: goto cellar");
    }

    void checkHealth(int expectedLevel){
        String response;
        response = sendCommandToServer("simon: health");
        response = response.toLowerCase();
        for(int i=0; i<=3; i++){
            if(i == expectedLevel) assertTrue(response.contains(String.valueOf(i)),
                    "Fight action may has not been executed correctly, " + response);
            else assertFalse(response.contains(String.valueOf(i)),
                    "Fight action may has not been executed correctly, should be " + expectedLevel + " rather " + i);
        }
    }

    // A lot of tests will probably check the game state using 'look' - so we better make sure 'look' works well !
    @Test
    void testLook() {
        String response = sendCommandToServer("simon: look");
        response = response.toLowerCase();
        assertTrue(response.contains("cabin"), "Did not see the name of the current room in response to look");
        assertTrue(response.contains("log cabin"), "Did not see a description of the room in response to look");
        assertTrue(response.contains("magic potion"), "Did not see a description of artifacts in response to look");
        assertTrue(response.contains("wooden trapdoor"), "Did not see description of furniture in response to look");
        assertTrue(response.contains("forest"), "Did not see available paths in response to look");
    }

    // Test that we can pick something up and that it appears in our inventory
    @Test
    void testGet()
    {
        String response;
        sendCommandToServer("simon: get potion");
        response = sendCommandToServer("simon: inv");
        response = response.toLowerCase();
        assertTrue(response.contains("potion"), "Did not see the potion in the inventory after an attempt was made to get it");
        response = sendCommandToServer("simon: look");
        response = response.toLowerCase();
        assertFalse(response.contains("potion"), "Potion is still present in the room after an attempt was made to get it");
    }

    // Test that we can goto a different location (we won't get very far if we can't move around the game !)
    @Test
    void testGoto()
    {
        sendCommandToServer("simon: goto forest");
        String response = sendCommandToServer("simon: look");
        response = response.toLowerCase();
        assertTrue(response.contains("key"), "Failed attempt to use 'goto' command to move to the forest - there is no key in the current location");
    }

    // Test that we can check our inventory and it could be altered correctly
    @Test
    void testInv()
    {
        String response;
        sendCommandToServer("simon: get potion");
        response = sendCommandToServer("simon: inv");
        response = response.toLowerCase();
        assertTrue(response.contains("potion"), "Did not see the potion in the inventory after an attempt was made to get it");
        sendCommandToServer("simon: drop the potion");
        response = sendCommandToServer("simon: inv");
        response = response.toLowerCase();
        assertFalse(response.contains("potion"), "Still can see the potion in the inventory after an attempt was made to drop it");
        response = sendCommandToServer("simon: look");
        response = response.toLowerCase();
        assertTrue(response.contains("potion"), "Did not see the potion in the current location after an attempt was made to drop it");
    }

    // Test that we can drop something and that it appears in the current location rather than our inventory
    @Test
    void testDrop()
    {
        String response;
        sendCommandToServer("simon: get potion");
        response = sendCommandToServer("simon: inv");
        response = response.toLowerCase();
        assertTrue(response.contains("potion"),
                "Potion is still present in the room after an attempt was made to get it somewhere else");
        sendCommandToServer("simon: goto cabin");
        response = sendCommandToServer("simon: look");
        response = response.toLowerCase();
        assertFalse(response.contains("potion"),
                "Potion is still present in the room after an attempt was made to drop it somewhere else");
        sendCommandToServer("simon: drop the potion");
    }

    @Test
    void testHealth()
    {
        String response;
        response = sendCommandToServer("simon: health");
        response = response.toLowerCase();
        assertTrue(response.contains("3"),
                "Player's current health level (as a number) hasn't been reported back correctly");
        assertFalse(response.contains("2"),
                "Player's current health level (as a number) hasn't been reported back correctly");
        assertFalse(response.contains("1"),
                "Player's current health level (as a number) hasn't been reported back correctly");
        assertFalse(response.contains("0"),
                "Player's current health level (as a number) hasn't been reported back correctly");
    }

    // Test custom action could be executed correctly
    @Test
    void testOpen()
    {
        String response;
        sendCommandToServer("simon: goto forest");
        sendCommandToServer("simon: get the key");
        sendCommandToServer("simon: goto cabin");
        sendCommandToServer("simon: goto cellar");
        response = sendCommandToServer("simon: look");
        response = response.toLowerCase();
        assertFalse(response.contains("cellar"), "Player reached the place where they are not supposed to");
        assertFalse(response.contains("elf"), "Player saw things that belong to the place they are not supposed to reach");
        // unlock the door
        sendCommandToServer("simon: open the trapdoor");
        sendCommandToServer("simon: goto cellar");
        response = sendCommandToServer("simon: look");
        response = response.toLowerCase();
        assertTrue(response.contains("cellar"), "Player did not reach the place where they should be able to");
        assertTrue(response.contains("elf"), "Player did not saw things that belong to where they are supposed to be");
    }

    @Test
    void testCut()
    {
        String response;
        sendCommandToServer("simon: goto forest");
        sendCommandToServer("simon: get log");
        response = sendCommandToServer("simon: inv");
        response = response.toLowerCase();
        assertFalse(response.contains("log"), "Log is picked up before the tree was cut down");
        // cut down the tree
        sendCommandToServer("simon: cut down the tree with an axe");
        sendCommandToServer("simon: get log");
        response = sendCommandToServer("simon: inv");
        response = response.toLowerCase();
        assertTrue(response.contains("log"), "Did not see the log in the forest after an attempt was made to cut tree down");
    }

    @Test
    void testFight()
    {
        String response;
        cabinToCellar();
        // check health level before fight
        response = sendCommandToServer("simon: health");
        response = response.toLowerCase();
        assertTrue(response.contains("3"),
                "Player's current health level (as a number) hasn't been reported back correctly");
        response = sendCommandToServer("simon: fight with the elf");
        response = response.toLowerCase();
        assertTrue(response.contains("health"), "Fight action may has not been executed correctly");
        // check health level after fight
        response = sendCommandToServer("simon: health");
        response = response.toLowerCase();
        System.out.println(response);
        checkHealth(2);
    }
}
