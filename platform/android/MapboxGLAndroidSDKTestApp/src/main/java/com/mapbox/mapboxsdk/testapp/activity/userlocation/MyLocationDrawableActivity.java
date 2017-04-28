package com.mapbox.mapboxsdk.testapp.activity.userlocation;

import android.graphics.Color;
import android.location.Location;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.content.ContextCompat;
import android.view.View;
import android.view.ViewGroup;

import com.mapbox.mapboxsdk.camera.CameraUpdateFactory;
import com.mapbox.mapboxsdk.constants.Style;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.location.LocationSource;
import com.mapbox.mapboxsdk.maps.MapView;
import com.mapbox.mapboxsdk.maps.MapboxMap;
import com.mapbox.mapboxsdk.maps.MapboxMapOptions;
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback;
import com.mapbox.mapboxsdk.testapp.R;
import com.mapbox.services.android.telemetry.location.LocationEngineListener;

/**
 * Test activity showcasing how to change the MyLocationView drawable.
 */
public class MyLocationDrawableActivity extends BaseLocationActivity implements LocationEngineListener {

  private MapView mapView;
  private MapboxMap mapboxMap;

  @Override
  protected void onCreate(@Nullable Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_my_location_customization);

    findViewById(R.id.progress).setVisibility(View.GONE);

    MapboxMapOptions mapboxMapOptions = new MapboxMapOptions();
    mapboxMapOptions.styleUrl(Style.MAPBOX_STREETS);

    // configure MyLocationView drawables
    mapboxMapOptions.myLocationForegroundDrawable(ContextCompat.getDrawable(this, R.drawable.ic_android));
    mapboxMapOptions.myLocationBackgroundDrawable(ContextCompat.getDrawable(this, R.drawable.ic_android));
    mapboxMapOptions.myLocationForegroundTintColor(Color.GREEN);
    mapboxMapOptions.myLocationBackgroundTintColor(Color.YELLOW);
    mapboxMapOptions.myLocationBackgroundPadding(new int[] {0, 0,
      (int) getResources().getDimension(R.dimen.locationview_background_drawable_padding),
      (int) getResources().getDimension(R.dimen.locationview_background_drawable_padding)});

    mapboxMapOptions.myLocationAccuracyTint(Color.RED);
    mapboxMapOptions.myLocationAccuracyAlpha(155);

    mapView = new MapView(this, mapboxMapOptions);
    mapView.setId(R.id.mapView);
    ViewGroup parent = (ViewGroup) findViewById(R.id.container);
    parent.addView(mapView);

    mapView.onCreate(savedInstanceState);
    mapView.getMapAsync(new OnMapReadyCallback() {
      @Override
      public void onMapReady(MapboxMap map) {
        mapboxMap = map;
        toggleGps(true);
      }
    });
  }

  @Override
  protected void enableLocation(boolean enabled) {
    if (enabled) {
      mapboxMap.setMyLocationEnabled(true);
      Location location = mapboxMap.getMyLocation();
      if (location != null) {
        onLocationChanged(location);
      } else {
        LocationSource.getLocationEngine(this).addLocationEngineListener(this);
      }
    } else {
      mapboxMap.setMyLocationEnabled(false);
    }
  }

  @Override
  public void onConnected() {
    // Nothing
  }

  @Override
  public void onLocationChanged(Location location) {
    if (mapboxMap != null) {
      mapboxMap.moveCamera(CameraUpdateFactory.newLatLngZoom(new LatLng(location), 14));
    }
  }

  @Override
  protected void onStart() {
    super.onStart();
    mapView.onStart();
  }

  @Override
  protected void onResume() {
    super.onResume();
    mapView.onResume();
  }

  @Override
  protected void onPause() {
    super.onPause();
    mapView.onPause();
  }

  @Override
  protected void onStop() {
    super.onStop();
    mapView.onStop();
  }

  @Override
  protected void onSaveInstanceState(Bundle outState) {
    super.onSaveInstanceState(outState);
    mapView.onSaveInstanceState(outState);
  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
    mapView.onDestroy();
  }

  @Override
  public void onLowMemory() {
    super.onLowMemory();
    mapView.onLowMemory();
  }
}
