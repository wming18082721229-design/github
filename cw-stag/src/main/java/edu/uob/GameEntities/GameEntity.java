package edu.uob.GameEntities;

 /*
    Different Types of Entity
    - Character: A creature/person involved in game
    - Player: A special kind of character (the user !)
    - Location: A room or place within the game
    - Artefact: A physical "thing" within the game (these things CAN be collected by the player)
    - Furniture: A physical "thing", part of a location (these things CANNOT be collected by the player)
 */

public abstract class GameEntity
{
    private String name;
    private String description;

    public GameEntity(String name, String description)
    {
        this.name = name;
        this.description = description;
    }

    public String getName()
    {
        return name;
    }

    public String getDescription()
    {
        return description;
    }
}
