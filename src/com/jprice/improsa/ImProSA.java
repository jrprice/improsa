package com.jprice.improsa;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;

import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

public class ImProSA extends Activity implements Spinner.OnItemSelectedListener
{
  Bitmap bmpInput, bmpOutput;
  ImageView imageResult;
  int width, height;
  int filterIndex;

  private static final int METHOD_REFERENCE  = (1<<0);
  private static final int METHOD_HALIDE_CPU = (1<<1);
  private static final int METHOD_HALIDE_GPU = (1<<2);
  private static final int METHOD_OPENCL     = (1<<3);

  static
  {
    System.loadLibrary("improsa");
  }
  private static native void process(
    Bitmap in, Bitmap out,
    int index, int type,
    int w, int h);
  private static native String[] getFilterList();

  @Override
  public void onCreate(Bundle savedInstanceState)
  {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.main);

    // Load source image
    bmpInput = BitmapFactory.decodeResource(
      this.getResources(),
      R.drawable.baboon);
    width = bmpInput.getWidth();
    height = bmpInput.getHeight();

    // Allocate image for result
    bmpOutput = Bitmap.createBitmap(width, height, bmpInput.getConfig());

    imageResult = (ImageView)findViewById(R.id.imageResult);
    imageResult.setImageBitmap(bmpInput);

    // Initialise filter list
    String[] filters = getFilterList();
    ArrayAdapter<String> adapter = new ArrayAdapter<String>(
      this, android.R.layout.simple_spinner_item, filters);
    adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
    Spinner filterSpinner = (Spinner)findViewById(R.id.filterSpinner);
    filterSpinner.setAdapter(adapter);
    filterSpinner.setOnItemSelectedListener(this);
    filterIndex = 0;
  }

  @Override
  public void onItemSelected(AdapterView<?> parent, View view,
                             int position, long id)
  {
    filterIndex = position;
  }

  @Override
  public void onNothingSelected(AdapterView<?> parent)
  {
    filterIndex = -1;
  }

  public void onReset(View view)
  {
    imageResult.setImageBitmap(bmpInput);
  }

  public void onRun(View view)
  {
    int id = view.getId();
    int method;
    switch (id)
    {
      case R.id.runReference:
        method = METHOD_REFERENCE;
        break;
      case R.id.runHalideCPU:
        method = METHOD_HALIDE_CPU;
        break;
      case R.id.runHalideGPU:
        method = METHOD_HALIDE_GPU;
        break;
      case R.id.runOpenCL:
        method = METHOD_OPENCL;
        break;
      default:
        return;
    }
    process(bmpInput, bmpOutput, filterIndex, method, width, height);
    imageResult.setImageBitmap(bmpOutput);
  }
}
