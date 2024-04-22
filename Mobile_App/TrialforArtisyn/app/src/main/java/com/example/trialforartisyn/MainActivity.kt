package com.example.trialforartisyn

import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.widget.Button
import android.widget.SeekBar
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat


class MainActivity : AppCompatActivity() {


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val bluetoothManager: BluetoothManager = getSystemService(BluetoothManager::class.java)
        val bluetoothAdapter: BluetoothAdapter? = bluetoothManager.getAdapter()
        val bluetoothLeScanner = bluetoothAdapter?.bluetoothLeScanner

        if (bluetoothAdapter == null) {
            println("Error: No Bluetooth Compatability")
        }
        else {
            println("Bluetooth Available")
        }
        val scanCallback: ScanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            val device = result.device
            println("RESULT: " + result)
            println("DEVICE: " + device)
            // ...do whatever you want with this found device
        }

        override fun onBatchScanResults(results: List<ScanResult>) {
            // Ignore for now
            println("RESULTS: " + results)
        }

        override fun onScanFailed(errorCode: Int) {
            // Ignore for now
            println("Scan Failed: " + errorCode)
        }
    }

        /*
       val activityResultLaunch = registerForActivityResult<Intent, ActivityResult>(
            ActivityResultContracts.StartActivityForResult(),
            ActivityResultCallback<ActivityResult?> {
                fun onActivityResult(result: ActivityResult) {
                    if (result.resultCode == 123) {
                        println("GOOD: " + result)
                    } else if (result.resultCode == 321) {
                        println("BAD: " + result)
                    }
                }
            })



        if (bluetoothAdapter?.isEnabled == false) {
            println("AJHSJKDHAJKSHDJKA HDAKJBDASHDKJAHSJLKDHASKJHDAKSJHDHKASJNDASBDASHDHASLKDHLASKHDKJASHDLKAJSHDLKJASHDLKJASHDLKJASHDLKAJSHDKLASHDKJLASHD")
            val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
            activityResultLaunch.launch(intent);
        }
        val discoverDevicesIntent = IntentFilter(BluetoothDevice.ACTION_FOUND)
        registerReceiver(receiver, discoverDevicesIntent)
        if (ActivityCompat.checkSelfPermission(
                this,
                Manifest.permission.BLUETOOTH_SCAN
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            if (bluetoothAdapter != null) {
                println("TRYING")
                scanLeDevice()
            }
        }*/




        val slider1: SeekBar = findViewById(R.id.slider1)
        val slider2: SeekBar = findViewById(R.id.slider2)
        val slider3: SeekBar = findViewById(R.id.slider3)
        val slider4: SeekBar = findViewById(R.id.slider4)
        val slider5: SeekBar = findViewById(R.id.slider5)
        val slider6: SeekBar = findViewById(R.id.slider6)
        val slider7: SeekBar = findViewById(R.id.slider7)
        val slider8: SeekBar = findViewById(R.id.slider8)
        val slider9: SeekBar = findViewById(R.id.slider9)
        val slider10: SeekBar = findViewById(R.id.slider10)
        val slider11: SeekBar = findViewById(R.id.slider11)
        val slider12: SeekBar = findViewById(R.id.slider12)
        val sliderVol: SeekBar = findViewById(R.id.sliderVolume)

        val submitButton: Button = findViewById(R.id.submitButton)

        submitButton.setOnClickListener {
            val value1 = slider1.progress
            val value2 = slider2.progress
            val value3 = slider3.progress
            val value4 = slider4.progress
            val value5 = slider5.progress
            val value6 = slider6.progress
            val value7 = slider7.progress
            val value8 = slider8.progress
            val value9 = slider9.progress
            val value10 = slider10.progress
            val value11 = slider11.progress
            val value12 = slider12.progress
            val volume = sliderVol.progress

            println("Volume: $volume")
            println("Slider 1: $value1")
            println("Slider 2: $value2")
            println("Slider 3: $value3")
            println("Slider 4: $value4")
            println("Slider 5: $value5")
            println("Slider 6: $value6")
            println("Slider 7: $value7")
            println("Slider 8: $value8")
            println("Slider 9: $value9")
            println("Slider 10: $value10")
            println("Slider 11: $value11")
            println("Slider 12: $value12")

            if( bluetoothLeScanner != null) {
                if (ActivityCompat.checkSelfPermission(
                        this,
                        Manifest.permission.BLUETOOTH_SCAN
                    ) != PackageManager.PERMISSION_GRANTED
                ) {
                    // TODO: Consider calling
                    //    ActivityCompat#requestPermissions
                    // here to request the missing permissions, and then overriding
                    //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
                    //                                          int[] grantResults)
                    // to handle the case where the user grants the permission. See the documentation
                    // for ActivityCompat#requestPermissions for more details.

                }
                bluetoothLeScanner.startScan(scanCallback)
                println("Scan Started")
            }
        }
    }
    /*
    private val receiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            println("Trying 2")
            val action = intent.action
            if (BluetoothDevice.ACTION_FOUND == action) {
                val device: BluetoothDevice? = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                // Add the device to a list or display its name and address
                println("Device: " + device)
            }
        }
    }*/

    override fun onDestroy() {
        super.onDestroy()
        //unregisterReceiver(receiver)
    }
}