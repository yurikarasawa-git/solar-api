package com.solar.api.service;

import com.solar.api.model.SensorData;
import com.solar.api.repository.SensorDataRepository;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class SensorService {

    private final SensorDataRepository repository;

    public SensorService(SensorDataRepository repository) {
        this.repository = repository;
    }

    public SensorData save(SensorData data) {
        return repository.save(data);
    }

    public List<SensorData> getLatest(String location) {
        if (location != null && !location.isBlank())
            return repository.findTop100ByLocationOrderByRecordedAtDesc(location);
        return repository.findTop100ByOrderByRecordedAtDesc();
    }
}
