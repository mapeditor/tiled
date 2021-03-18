package org.mapeditor.util;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Unmarshaller;

public class UnmarshallerPool extends Pool<Unmarshaller> {
    private final JAXBContext context;

    public UnmarshallerPool(JAXBContext context) {
        this.context = context;
    }

    @Override
    protected Unmarshaller create() {
        try {
            return context.createUnmarshaller();
        }
        catch (JAXBException e) {
            throw new RuntimeException(e);
        }
    }
}
