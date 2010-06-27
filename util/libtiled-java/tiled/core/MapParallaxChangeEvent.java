/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package tiled.core;

/**
 *
 * @author upachler
 */
public class MapParallaxChangeEvent {

    public ChangeType getChangeType() {
        return changeType;
    }

    public Map getMap() {
        return map;
    }

    public int getLayerIndex() {
        return layerIndex;
    }

    public enum ChangeType{
        /// indicates that the map's eye/viewplane distance changed.
        EYE_VIEWPLANE_DISTANCE,
        /// indicates that a layer's viewplane distance changed. This
        /// may include that it has been set to infinity
        LAYER_VIEWPLANE_DISTANCE,
    }
    
    private ChangeType changeType;
    private Map map;
    private int layerIndex = -1;
    
    public MapParallaxChangeEvent(Map map, int layerIndex, ChangeType changeType){
        this.map = map;
        this.changeType = changeType;
        if(changeType != ChangeType.EYE_VIEWPLANE_DISTANCE)
            this.layerIndex = layerIndex;
    }
}
