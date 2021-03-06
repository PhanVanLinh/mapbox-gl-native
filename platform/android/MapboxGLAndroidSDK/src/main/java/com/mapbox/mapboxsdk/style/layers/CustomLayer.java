package com.mapbox.mapboxsdk.style.layers;

/**
 * Custom layer.
 * <p>
 * Experimental feature. Do not use.
 * </p>
 */
public class CustomLayer extends Layer {

  public CustomLayer(String id,
                     long context,
                     long initializeFunction,
                     long renderFunction,
                     long deinitializeFunction) {
    initialize(id, initializeFunction, renderFunction, deinitializeFunction, context);
  }

  public CustomLayer(long nativePtr) {
    super(nativePtr);
  }

  public void update() {
    nativeUpdate();
  }

  protected native void initialize(String id, long initializeFunction, long renderFunction, long deinitializeFunction,
                                   long context);

  protected native void nativeUpdate();

  @Override
  protected native void finalize() throws Throwable;

}
