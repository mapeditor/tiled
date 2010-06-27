/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package tiled.core;

/**
 * A change event for a layer specifies what change happened to that layer.
 * To know the type of change, call getChangeType(). Depending on the value
 * it returns, certain member functions will yield valid values (look at
 * the documentation for the <code>CHANGETYPE_*</code> constants for details).
 * @author upachler
 */
public class MapLayerChangeEvent {
    
    /**
     * Indicates that the layer in question has been renamed. The 
     * getOldName() and getNewName() member functions will yield the layer's
     * old and new name.
     */
    public static final int CHANGETYPE_NAME = 1;
    
    private int changeType = -1;
    
    private String oldName;
    private String newName;
    
    private MapLayerChangeEvent(int changeType){
        this.changeType = changeType;
    }
        
    
    static MapLayerChangeEvent createNameChangeEvent(String oldName, String newName){
        MapLayerChangeEvent e = new MapLayerChangeEvent(CHANGETYPE_NAME);
        e.oldName = oldName;
        e.newName = newName;
        return e;
    }

    public int getChangeType() {
        return changeType;
    }

    public String getOldName() {
        return oldName;
    }

    public String getNewName() {
        return newName;
    }
}
