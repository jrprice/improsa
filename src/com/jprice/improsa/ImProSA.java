package com.jprice.improsa;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
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
  ProcessTask processTask = null;

  private static final String TAG = "improsa";

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

    // Check for command-line arguments to auto-run filters
    String filter = getIntent().getStringExtra("FILTER");
    String method = getIntent().getStringExtra("METHOD");
    if (filter != null && method != null && savedInstanceState == null)
    {
      // Find filter index
      filterIndex = -1;
      for (int i = 0; i < filters.length; i++)
      {
        if (filters[i].equalsIgnoreCase(filter))
        {
          filterIndex = i;
          break;
        }
      }

      if (filterIndex == -1)
      {
        Log.d(TAG, "Filter '" + filter + "' not found.");
        filterIndex = 0;
      }
      else
      {
        filterSpinner.setSelection(filterIndex);

        // Parse method
        if (method.equalsIgnoreCase("reference"))
        {
          run(METHOD_REFERENCE);
        }
        else if (method.equalsIgnoreCase("halide_cpu"))
        {
          run(METHOD_HALIDE_CPU);
        }
        else if (method.equalsIgnoreCase("halide_gpu"))
        {
          run(METHOD_HALIDE_GPU);
        }
        else if (method.equalsIgnoreCase("opencl"))
        {
          run(METHOD_OPENCL);
        }
        else
        {
          Log.d(TAG, "Invalid method '" + method + "'.");
        }
      }
    }
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
    switch (view.getId())
    {
      case R.id.runReference:
        run(METHOD_REFERENCE);
        break;
      case R.id.runHalideCPU:
        run(METHOD_HALIDE_CPU);
        break;
      case R.id.runHalideGPU:
        run(METHOD_HALIDE_GPU);
        break;
      case R.id.runOpenCL:
        run(METHOD_OPENCL);
        break;
      default:
        return;
    }
  }

  public void run(int method)
  {
    if (processTask != null &&
        processTask.getStatus() != AsyncTask.Status.FINISHED)
    {
      return;
    }
    processTask = new ProcessTask(method);
    processTask.execute();
  }

  private class ProcessTask extends AsyncTask<Void, String, Boolean>
  {
    private native boolean process(
      Bitmap in, Bitmap out,
      int index, int type,
      int w, int h);

    private int filterMethod;
    private boolean success;

    public ProcessTask(int method)
    {
      filterMethod = method;
    }

    @Override
    protected Boolean doInBackground(Void... inputs)
    {
      return process(bmpInput, bmpOutput, filterIndex, filterMethod, width, height);
    }

    @Override
    protected void onPostExecute(Boolean result)
    {
      imageResult.setImageBitmap(bmpOutput);
      imageResult.setBackgroundColor(result ? 0xFF00FF00 : 0xFFFF0000);
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
