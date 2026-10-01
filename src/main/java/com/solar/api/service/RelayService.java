package com.solar.api.service;

import com.solar.api.model.RelayState;
import com.solar.api.repository.RelayStateRepository;
import jakarta.annotation.PostConstruct;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class RelayService {

    private final RelayStateRepository repository;

    public RelayService(RelayStateRepository repository) {
        this.repository = repository;
    }

    @PostConstruct
    public void init() {
        seedRelay(1, "2F", "Relay 1", false);
        seedRelay(2, "2F", "Relay 2", false);
        seedRelay(3, "2F", "Relay 3", false);
        seedRelay(4, "2F", "Relay 4", false);
        seedRelay(5, "1F", "Inverter", true);
        seedRelay(6, "1F", "Relay 6",  false);
    }

    private void seedRelay(int number, String location, String label, boolean isInverter) {
        if (repository.findById(number).isEmpty()) {
            RelayState relay = new RelayState();
            relay.setRelayNumber(number);
            relay.setState(false);
            relay.setLocation(location);
            relay.setLabel(label);
            relay.setIsInverter(isInverter);
            repository.save(relay);
        }
    }

    public List<RelayState> getAllStates(String location) {
        if (location != null && !location.isBlank())
            return repository.findByLocationOrderByRelayNumber(location);
        return repository.findAll();
    }

    public RelayState setState(int relayNumber, boolean state) {
        RelayState relay = repository.findById(relayNumber)
                .orElseThrow(() -> new IllegalArgumentException("Relay not found: " + relayNumber));
        relay.setState(state);
        return repository.save(relay);
    }
}
