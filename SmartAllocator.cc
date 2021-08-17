#include <iostream>
#include <cstdint>
#include <forward_list>

using namespace std;

const unsigned int guSize = 1048576;

template<typename DataType>
class SmartMemoryControlBlock {
 private:
  DataType *m_object_ptr = nullptr;
  uint_fast32_t m_length = 0; // or size
  bool m_is_available = false;
 public:
  [[nodiscard]] bool IsAvailable() const {
    return m_is_available;
  }

  [[nodiscard]] uint_fast32_t GetSize() const {
    return m_length;
  }

  void SetSize(DataType new_length_value) {
    m_length = new_length_value;
  }

  void SetAvailability(bool new_availability_value) {
    m_is_available = new_availability_value;
  }

  DataType *operator->() {
    if (m_object_ptr == nullptr) {
      m_object_ptr = new DataType();
      return m_object_ptr;
    }
    return m_object_ptr;
  }

  explicit operator DataType *() {
    if (m_object_ptr == nullptr) {
      m_object_ptr = new DataType();
      return m_object_ptr;
    }
    return m_object_ptr;
  }

  explicit SmartMemoryControlBlock(DataType *Pointer) : m_object_ptr(Pointer == nullptr ? new DataType() : Pointer) {}

  ~SmartMemoryControlBlock() {
    if (m_object_ptr == nullptr) delete m_object_ptr;
  }
};

class SmallAllocator {
 private:
  SmartMemoryControlBlock<uint_fast8_t> *m_memory_control_block_ = nullptr;
  uint_fast8_t m_has_initialized = 0;
  uint_fast8_t m_memory[guSize] = {0};

  uint_fast8_t *m_start = nullptr;
  uint_fast8_t *m_current_position = nullptr;
  uint_fast8_t *m_last_valid_address = nullptr;
  uint_fast8_t *m_end = m_memory + sizeof(uint_fast8_t) * guSize;

 public:
  void *Alloc(uint_fast32_t Size) {
    void *memory_location = nullptr;
    if (!m_has_initialized) {
      SmallAllocator();
    }

    if (Size > *m_end) {
      cout << "We are fucked up, bro: no more memory" << endl;
      return m_end;
    }

    Size = Size + sizeof(class SmartMemoryControlBlock<uint_fast8_t>);
    m_current_position = m_start;

    while (m_current_position != m_last_valid_address) {
      m_memory_control_block_ = new SmartMemoryControlBlock(m_current_position);

      if (m_memory_control_block_->IsAvailable()) {
        if (m_memory_control_block_->GetSize() >= Size) {
          m_memory_control_block_->SetAvailability(false);
          memory_location = m_current_position;
          break;
        }
      }
      m_current_position = m_current_position + m_memory_control_block_->GetSize();
    }
    if (memory_location == nullptr) {
      m_current_position = &m_memory[Size];
      memory_location = m_last_valid_address;
      m_last_valid_address = m_last_valid_address + Size;

      m_memory_control_block_ = new SmartMemoryControlBlock(static_cast<uint_fast8_t *>(memory_location));
      m_memory_control_block_->SetAvailability(false);
      m_memory_control_block_->SetSize(Size);
    }
    *reinterpret_cast<uint_fast8_t *>(memory_location) += m_memory_control_block_->GetSize();

    return static_cast<void*>(memory_location);
  };

  void *ReAlloc(void *Pointer, uint_fast32_t Size) {
    void *memory_location = nullptr;
    if (!m_has_initialized) {
      SmallAllocator();
    }
    Size = Size + sizeof(class SmartMemoryControlBlock<uint_fast8_t>);
    m_current_position = m_start;

    if (m_memory_control_block_ == nullptr) return m_current_position;
    while (m_memory_control_block_->GetSize() != Size) {
      m_memory_control_block_ = new SmartMemoryControlBlock(m_current_position);

      if (m_memory_control_block_->IsAvailable()) {
        if (m_memory_control_block_->GetSize() >= Size) {
          m_memory_control_block_->SetAvailability(false);
          memory_location = m_current_position;
          break;
        }
      }
      m_current_position = m_current_position + m_memory_control_block_->GetSize();
    }
    memory_location = m_last_valid_address;
  };

  void Free(void *Pointer) {
    SmartMemoryControlBlock<uint_fast8_t> *restored_memory_block;
    restored_memory_block = reinterpret_cast<SmartMemoryControlBlock<uint_fast8_t> *>(Pointer) - m_memory_control_block_->GetSize();
    restored_memory_block->SetAvailability(true);
  };

  SmallAllocator() {
    m_current_position = &m_memory[0];
    m_start = m_current_position;
    m_last_valid_address = m_current_position;
    m_has_initialized = 1;
  }
};

int main() {
  SmallAllocator A1;

  auto *A1_P1 = (int_fast32_t *) A1.Alloc(sizeof(int_fast32_t));
  A1_P1 = (int_fast32_t *) A1.ReAlloc(A1_P1, 2 * sizeof(int_fast32_t));
  A1.Free(A1_P1);

  SmallAllocator A2;
  auto *A2_P1 = (int_fast32_t *) A2.Alloc(10 * sizeof(int_fast32_t));

  for (auto i = 0; i < 10; i++) A2_P1[i] = i;
  for (auto i = 0; i < 10; i++) if (A2_P1[i] != i) std::cout << "ERROR 1" << std::endl;

  auto *A2_P2 = (int_fast32_t *) A2.Alloc(10 * sizeof(int_fast32_t));

  for (auto i = 0; i < 10; i++) A2_P2[i] = -1;
  for (auto i = 0; i < 10; i++) if (A2_P1[i] != i) std::cout << "ERROR 2" << std::endl;
  for (auto i = 0; i < 10; i++) if (A2_P2[i] != -1) std::cout << "ERROR 3" << std::endl;

  A2_P1 = (int_fast32_t *) A2.ReAlloc(A2_P1, 20 * sizeof(int_fast32_t));

  for (auto i = 10; i < 20; i++) A2_P1[i] = i;
  for (auto i = 0; i < 20; i++) if (A2_P1[i] != i) std::cout << "ERROR 4" << std::endl;
  for (auto i = 0; i < 10; i++) if (A2_P2[i] != -1) std::cout << "ERROR 5" << std::endl;

  A2_P1 = (int_fast32_t *) A2.ReAlloc(A2_P1, 5 * sizeof(int_fast32_t));

  for (auto i = 0; i < 5; i++) if (A2_P1[i] != i) std::cout << "ERROR 6" << std::endl;
  for (auto i = 0; i < 10; i++) if (A2_P2[i] != -1) std::cout << "ERROR 7" << std::endl;

  A2.Free(A2_P1);
  A2.Free(A2_P2);

  return 0;
}