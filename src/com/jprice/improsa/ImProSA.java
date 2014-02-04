package com.jprice.improsa;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.widget.ImageView;

public class ImProSA extends Activity
{
  Bitmap original;
  ImageView imageResult;

  /** Called when the activity is first created. */
  @Override
  public void onCreate(Bundle savedInstanceState)
  {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.main);

    // Load source image
    original = BitmapFactory.decodeResource(
      this.getResources(),
      R.drawable.baboon);

    imageResult = (ImageView)findViewById(R.id.imageResult);
    imageResult.setImageBitmap(original);
  }
}
