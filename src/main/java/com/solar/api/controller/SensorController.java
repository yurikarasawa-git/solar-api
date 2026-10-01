package com.solar.api.controller;

import com.solar.api.model.SensorData;
import com.solar.api.service.SensorService;
import jakarta.validation.Valid;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/api/sensor")
@CrossOrigin(origins = "*")
public class SensorController {

    private final SensorService service;

    public SensorController(SensorService service) {
        this.service = service;
    }

    // ESP32 posts: { "temperature": 25.3, "chargingCurrent": 2.1, "batteryVoltage": 12.6 }
    @PostMapping
    public ResponseEntity<SensorData> receive(@Valid @RequestBody SensorData data) {
        return ResponseEntity.ok(service.save(data));
    }

    // Frontend fetches latest 100 readings; optional ?location=1F filter
    @GetMapping
    public ResponseEntity<List<SensorData>> getLatest(
            @RequestParam(required = false) String location) {
        return ResponseEntity.ok(service.getLatest(location));
    }
}
