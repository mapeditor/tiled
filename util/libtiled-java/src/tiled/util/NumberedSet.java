/*
 *  Tiled Map Editor, (c) 2004-2006
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Adam Turk <aturk@biggeruniverse.com>
 *  Bjorn Lindeijer <bjorn@lindeijer.nl>
 */

package tiled.util;

import java.util.Iterator;
import java.util.Vector;

/**
 * A NumberedSet is a generic container of Objects where each element is
 * identified by an integer id.  Unlike with a Vector, the mapping between
 * id and element remains unaffected when elements are deleted.  This means
 * that the set of ids for a NumberedSet may not be contiguous. (A sparse
 * array)
 *
 * @author rainerd
 */
public class NumberedSet
{
    private Vector<Object> data;

    /**
     * Constructs a new empty NumberedSet.
     */
    public NumberedSet() {
        data = new Vector<Object>();
    }

    /**
     * Returns the element for a specific element, or null if the id does not
     * identify any element in this NumberedSet.
     *
     * @param id
     * @return Object
     */
    public Object get(int id) {
        try {
            return data.get(id);
        } catch (ArrayIndexOutOfBoundsException e) {}

        return null;
    }

    /**
     * Returns true if the NumberedSet contains an element for the specified id.
     *
     * @param id
     * @return boolean
     */
    public boolean containsId(int id) {
        return get(id) != null;
    }

    /**
     * Sets the element for the specified id, replacing any previous element that
     * was associated with that id.  id should be a relatively small positive
     * integer.
     *
     * @param id
     * @param o
     * @return int
     * @throws IllegalArgumentException
     */
    public int put(int id, Object o) throws IllegalArgumentException {
        if (id < 0) throw new IllegalArgumentException();

        // Make sure there is sufficient space to overlay
        for (int i = id - data.size(); i > 0; i--) {
            data.add(null);
        }

        data.add(id, o);
        return id;
    }

    /**
     * Removes the element associated with the given id from the NumberedSet.
     *
     * todo: this function shifts the ids of any subsequent elements!
     *
     * @param id
     */
    public void remove(int id) {
        data.remove(id);
    }

    /**
     * Returns the last id in the NumberedSet that is associated with an
     * element, or -1 if the NumberedSet is empty.
     *
     * @return int
     */
    public int getMaxId() {
        int maxId = data.size() - 1;

        while (maxId >= 0) {
            if (data.get(maxId) != null) {
                break;
            }
            maxId--;
        }

        return maxId;
    }

    /**
     * Returns an iterator to iterate over the elements of the NumberedSet.
     *
     * @return NumberedSetIterator
     */
    public Iterator<Object> iterator() {
        return data.iterator();
    }

    /**
     * Adds a new element to the NumberedSet and returns its id.
     *
     * @param o
     * @return int
     */
    public int add(Object o) {
        int id = getMaxId() + 1;
        put(id, o);
        return id;
    }

    /**
     * Returns the id of the first element of the NumberedSet that is euqal to
     * the given object, or -1 otherwise.
     *
     * @param o
     */
    public int indexOf(Object o) {
        return data.indexOf(o);
    }

    /**
     * Returns true if at least one element of the NumberedSet is equal to the
     * given object.
     */
    public boolean contains(Object o) {
        return data.contains(o);
    }

    /**
     * If this NumberedSet already contains an element equal to the given object,
     * return its id.  Otherwise insert the given object into the NumberedSet
     * and return its id.
     */
    public int findOrAdd(Object o) {
        int id = indexOf(o);
        if (id != -1) return id;
        return add(o);
    }

    /**
     * Returns the number of actual elements in the NumberedSet.
     *
     * @return int
     */
    public int size() {
        return data.size();
    }
}
