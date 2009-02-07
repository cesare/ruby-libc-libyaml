require File.join(File.dirname(__FILE__), 'spec_helper')

require 'yaml'

describe YAML::LibYAML do
  test_data = <<-EOF_TEST_DATA
### scalar Fixnum
---
100

### scalar Oct
---
010

### scalar Hex
---
0x1f

### scalar Float
---
10.1

### nil
---
~

### scalar UTF8 String
---
あいうえお

### simple mapping
---
key: val

### inline mapping
---
{ key: val }

### simple sequense
---
- foo
- bar
- baz

### inline sequense
---
[ foo, bar, baz ]

### seq in map
---
global:
  - foo
  - bar
  - baz

### TODO: anchor
---
my_config1: &anchor1
 - foo
 - bar
 - baz

my_config2: *anchor1

# from yaml spec exmaple, see also http://yaml.org/spec/1.1/
### TODO: Example 2.19. integers
---
canonical: 12345
decimal: +12_345
sexagesimal: 3:25:45
octal: 014
hexadecimal: 0xC

### TODO: Example 2.20 Floating Point
---
canonical: 1.23015e+3
exponential: 12.3015e+02
sexagesimal: 20:30.15
fixed: 1_230.15
negative infinity: -.inf
not a number: .NaN

### TODO: Example 2.21 Miscelleneous
---
null: ~
true: boolean
false: boolean
string: '12345'

### TODO: Example 2.22 Timestamps
---
canonical: 2001-12-15T02:59:43.1Z
iso8601: 2001-12-14t21:59:43.10-05:00
spaced: 2001-12-14 21:59:43.10 -5
date: 2002-12-14

EOF_TEST_DATA

  describe '.load_stream' do
    it 'should be compatible to YAML' do
      violated "assertion failed"
      YAML::LibYAML.load_stream(test_data).documents.should == YAML.load_stream(test_data).documents
    end
  end

  describe ".load" do
    # making fixture data
    data = test_data.split(/^(###.+)$\n/)
    data.shift # remove garbage

    while message = data.shift
      message.gsub!(/^###\s+/, '')
      yaml = data.shift

      class_eval <<-EOF_EVAL
        it "should be compatible to YAML ( #{message} )" do
          YAML::LibYAML.load(#{yaml.inspect}).should == YAML.load(#{yaml.inspect})
        end
      EOF_EVAL
    end
  end
end

