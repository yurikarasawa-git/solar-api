package com.solar.api.repository;

import com.solar.api.model.SensorData;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public interface SensorDataRepository extends JpaRepository<SensorData, Long> {
    List<SensorData> findTop100ByOrderByRecordedAtDesc();
    List<SensorData> findTop100ByLocationOrderByRecordedAtDesc(String location);
}
