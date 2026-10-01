package com.solar.api.repository;

import com.solar.api.model.RelayState;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public interface RelayStateRepository extends JpaRepository<RelayState, Integer> {
    List<RelayState> findByLocationOrderByRelayNumber(String location);
}
