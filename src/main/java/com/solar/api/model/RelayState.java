package com.solar.api.model;

import jakarta.persistence.*;
import java.time.LocalDateTime;

@Entity
@Table(name = "relay_state")
public class RelayState {

    @Id
    private Integer relayNumber;

    @Column(nullable = false)
    private Boolean state;

    @Column(nullable = false)
    private String location;

    @Column(nullable = false)
    private String label;

    @Column(nullable = false)
    private Boolean isInverter = false;

    private LocalDateTime updatedAt;

    @PrePersist
    @PreUpdate
    protected void onUpdate() {
        updatedAt = LocalDateTime.now();
    }

    public Integer getRelayNumber() { return relayNumber; }
    public void setRelayNumber(Integer relayNumber) { this.relayNumber = relayNumber; }
    public Boolean getState() { return state; }
    public void setState(Boolean state) { this.state = state; }
    public String getLocation() { return location; }
    public void setLocation(String location) { this.location = location; }
    public String getLabel() { return label; }
    public void setLabel(String label) { this.label = label; }
    public Boolean getIsInverter() { return isInverter; }
    public void setIsInverter(Boolean isInverter) { this.isInverter = isInverter; }
    public LocalDateTime getUpdatedAt() { return updatedAt; }
}
