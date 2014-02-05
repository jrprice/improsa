package com.jprice.improsa;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Spinner;
import android.widget.TextView;

public class ImProSA extends Activity implements Spinner.OnItemSelectedListener
{
  Bitmap bmpInput, bmpOutput;
  ImageView imageResult;
  TextView status;
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
  private native String[] getFilterList();

  @Override
  public void onCreate(Bundle savedInstanceState)
  {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.main);

    // Load input image
    bmpInput = BitmapFactory.decodeResource(
      this.getResources(),
      R.drawable.baboon);
    width = bmpInput.getWidth();
    height = bmpInput.getHeight();

    // Allocate output image
    bmpOutput = Bitmap.createBitmap(width, height, bmpInput.getConfig());

    // Initialise image view
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

    // Initialize status text
    status = (TextView)findViewById(R.id.status);
    status.setText("Ready.");
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
    new ProcessTask(method).execute();
  }

  public void setButtonsEnabled(boolean enabled)
  {
    findViewById(R.id.reset).setEnabled(enabled);
    findViewById(R.id.runReference).setEnabled(enabled);
    findViewById(R.id.runHalideCPU).setEnabled(enabled);
    findViewById(R.id.runHalideGPU).setEnabled(enabled);
    findViewById(R.id.runOpenCL).setEnabled(enabled);
  }

  private class ProcessTask extends AsyncTask<Void, String, Void>
  {
    private native void process(
      Bitmap in, Bitmap out,
      int index, int type,
      int w, int h);

    int filterMethod;

    public ProcessTask(int method)
    {
      filterMethod = method;
    }

    @Override
    protected Void doInBackground(Void... inputs)
    {
      process(bmpInput, bmpOutput, filterIndex, filterMethod, width, height);
      return null;
    }

    @Override
    protected void onPostExecute(Void result)
    {
      imageResult.setImageBitmap(bmpOutput);
      setButtonsEnabled(true);
    }

    @Override
    protected void onPreExecute()
    {
      setButtonsEnabled(false);
    }

    @Override
    protected void onProgressUpdate(String... values)
    {
      if (values.length > 0)
      {
        status.setText(values[0]);
      }
    }

    public void updateStatus(String text)
    {
      publishProgress(text);
    }
  }
}
