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
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.Spinner;
import android.widget.TextView;

public class ImProSA extends Activity implements Spinner.OnItemSelectedListener
{
  Bitmap bmpInput, bmpOutput;
  ImageView imageResult;
  Spinner filterSpinner, imageSpinner;
  CheckBox verifyCheckBox, wgsizeCheckBox;
  Spinner wgxSpinner, wgySpinner;
  TextView status;
  int width, height;
  int filterIndex;
  ProcessTask processTask = null;

  private static final String TAG = "improsa";

  private static final int METHOD_REFERENCE  = (1<<1);
  private static final int METHOD_HALIDE_CPU = (1<<2);
  private static final int METHOD_HALIDE_GPU = (1<<3);
  private static final int METHOD_OPENCL     = (1<<4);

  static
  {
    System.loadLibrary("improsa");
  }
  private native void clearReferenceCache();
  private native String[] getFilterList();
  private native void setVerificationEnabled(boolean enabled);
  private native void setWorkGroupSize(int x, int y);

  @Override
  public void onCreate(Bundle savedInstanceState)
  {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.main);

    // Initialise image view
    imageResult = (ImageView)findViewById(R.id.imageResult);
    loadImage(R.drawable.baboon);

    imageResult.setImageBitmap(bmpInput);

    // Initialise filter spinner
    String[] filters = getFilterList();
    ArrayAdapter<String> adapter = new ArrayAdapter<String>(
      this, android.R.layout.simple_spinner_item, filters);
    adapter.setDropDownViewResource(
      android.R.layout.simple_spinner_dropdown_item);
    filterSpinner = (Spinner)findViewById(R.id.filterSpinner);
    filterSpinner.setAdapter(adapter);
    filterSpinner.setOnItemSelectedListener(this);
    filterIndex = 0;

    // Initialise image spinner
    String[] images =
    {
      "Peppers",
      "Baboon",
      "Lena",
      "Doughnuts"
    };
    adapter = new ArrayAdapter<String>(
      this, android.R.layout.simple_spinner_item, images);
    adapter.setDropDownViewResource(
      android.R.layout.simple_spinner_dropdown_item);
    imageSpinner = (Spinner)findViewById(R.id.imageSpinner);
    imageSpinner.setAdapter(adapter);
    imageSpinner.setOnItemSelectedListener(this);
    imageSpinner.setSelection(1);

    // Initialise work-group size spinners
    Integer[] wgsizes = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
    ArrayAdapter<Integer> intAdapter = new ArrayAdapter<Integer>(
      this, android.R.layout.simple_spinner_item, wgsizes);
    intAdapter.setDropDownViewResource(
      android.R.layout.simple_spinner_dropdown_item);
    wgxSpinner = (Spinner)findViewById(R.id.wgxSpinner);
    wgxSpinner.setAdapter(intAdapter);
    wgxSpinner.setOnItemSelectedListener(this);
    wgxSpinner.setEnabled(false);
    wgySpinner = (Spinner)findViewById(R.id.wgySpinner);
    wgySpinner.setAdapter(intAdapter);
    wgySpinner.setOnItemSelectedListener(this);
    wgySpinner.setEnabled(false);
    wgsizeCheckBox = ((CheckBox)findViewById(R.id.wgsize));
    wgsizeCheckBox.setChecked(false);
    setWorkGroupSize(0, 0);

    // Initialize verification checkbox
    verifyCheckBox = (CheckBox)findViewById(R.id.verify);
    verifyCheckBox.setChecked(true);
    setVerificationEnabled(true);

    // Initialize status text
    status = (TextView)findViewById(R.id.status);
    status.setText("Ready.");

    // Check for command-line arguments to auto-run filters
    if (savedInstanceState == null)
    {
      // Check for optional image selection
      String image = getIntent().getStringExtra("IMAGE");
      if (image != null)
      {
        if (image.equalsIgnoreCase("peppers"))
        {
          loadImage(R.drawable.peppers);
        }
        else if (image.equalsIgnoreCase("baboon"))
        {
          loadImage(R.drawable.baboon);
        }
        else if (image.equalsIgnoreCase("lena"))
        {
          loadImage(R.drawable.lena);
        }
        else if (image.equalsIgnoreCase("doughnuts"))
        {
          loadImage(R.drawable.doughnuts);
        }
        else
        {
          Log.d(TAG, "Invalid image '" + image + "'.");
          return;
        }
      }

      // Check for optional work-group size specification
      int[] wgsize = getIntent().getIntArrayExtra("WGSIZE");
      if (wgsize != null)
      {
        if (wgsize.length != 2)
        {
          Log.d(TAG, "Invalid work-group size.");
          return;
        }
        setWorkGroupSize(wgsize[0], wgsize[1]);
      }

      // Check for verification flag
      boolean verify = getIntent().getBooleanExtra("VERIFY", true);
      verifyCheckBox.setChecked(verify);
      setVerificationEnabled(verify);

      // Filter and method are required
      String filter = getIntent().getStringExtra("FILTER");
      String method = getIntent().getStringExtra("METHOD");
      if (filter != null && method != null)
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
  }

  public void loadImage(int id)
  {
    // Load input image
    bmpInput = BitmapFactory.decodeResource(this.getResources(), id);
    width = bmpInput.getWidth();
    height = bmpInput.getHeight();

    // Allocate output image
    bmpOutput = Bitmap.createBitmap(width, height, bmpInput.getConfig());

    // Update view
    imageResult.setImageBitmap(bmpInput);

    // Clear reference caches
    clearReferenceCache();
  }

  @Override
  public void onItemSelected(AdapterView<?> parent, View view,
                             int position, long id)
  {
    if (parent == filterSpinner)
    {
      filterIndex = position;
    }
    else if (parent == imageSpinner)
    {
      switch (position)
      {
      case 0:
        loadImage(R.drawable.peppers);
        break;
      case 1:
        loadImage(R.drawable.baboon);
        break;
      case 2:
        loadImage(R.drawable.lena);
        break;
      case 3:
        loadImage(R.drawable.doughnuts);
        break;
      default:
        return;
      }
    }
    else if ((parent == wgxSpinner || parent == wgySpinner) &&
             wgsizeCheckBox.isChecked())
    {
      setWorkGroupSize(
        (Integer)wgxSpinner.getSelectedItem(),
        (Integer)wgySpinner.getSelectedItem());
    }
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

  public void onVerifyChecked(View view)
  {
    setVerificationEnabled(verifyCheckBox.isChecked());
  }

  public void onWGSizeChecked(View view)
  {
    boolean checked = wgsizeCheckBox.isChecked();
    wgxSpinner.setEnabled(checked);
    wgySpinner.setEnabled(checked);
    if (checked)
    {
      setWorkGroupSize(
        (Integer)wgxSpinner.getSelectedItem(),
        (Integer)wgySpinner.getSelectedItem());
    }
    else
    {
      setWorkGroupSize(0, 0);
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
      return process(
        bmpInput, bmpOutput, filterIndex, filterMethod, width, height);
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
